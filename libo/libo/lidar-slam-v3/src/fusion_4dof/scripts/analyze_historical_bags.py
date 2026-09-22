#!/usr/bin/env python3
"""Read-only ROS1 bag audit; this is not a replay of the current fusion node.

Usage: python3 analyze_historical_bags.py path/to/*.bag
Requires: pip install rosbags
"""

import argparse
import bisect
import collections
import json
import math
import statistics
from pathlib import Path

from rosbags.highlevel import AnyReader


TOPICS = {
    "/Mower/position",
    "/Mower/debug/gps",
    "/Mower/debug/lio",
    "/Mower/lio_slam_level",
    "/nanobot/gpsposition",
    "/Mower/localization_status",
    "/Mower/localization_algorithm_status",
    "/mower/fusion_stop_car",
    "/mower/manager_stop_car",
}


def stamp(message):
    value = message.header.stamp
    return value.sec + value.nanosec * 1e-9


def percentile(values, fraction):
    if not values:
        return None
    ordered = sorted(values)
    return round(ordered[min(len(ordered) - 1, int(fraction * (len(ordered) - 1)))], 5)


def summarize(path):
    positions = []
    lio = []
    gps = []
    debug_gps = []
    debug_lio = []
    status = []
    alg_status = []
    stops = collections.defaultdict(list)
    with AnyReader([path]) as reader:
        connections = [c for c in reader.connections if c.topic in TOPICS]
        for connection, record_ns, raw in reader.messages(connections=connections):
            topic = connection.topic
            message = reader.deserialize(raw, connection.msgtype)
            record_time = record_ns * 1e-9
            if topic == "/Mower/position":
                positions.append((stamp(message), record_time, message.position_state,
                                  message.position_x, message.position_y, message.yaw))
            elif topic == "/Mower/lio_slam_level":
                lio.append((stamp(message), record_time, message.lio_state,
                            message.q_w, message.q_x, message.q_y, message.q_z))
            elif topic == "/nanobot/gpsposition":
                gps.append((stamp(message), record_time, message.gps_flag,
                            message.INS_Status, message.INS_GpsFlag_Pos,
                            message.INS_NumSV, message.INS_Gps_Age,
                            message.INS_Std_Lat, message.INS_Std_Lon,
                            message.INS_Std_LocatHeight,
                            message.latitude, message.longitude,
                            message.azimuth, message.positionStatus,
                            message.INS_Std_Heading))
            elif topic == "/Mower/debug/gps":
                debug_gps.append((stamp(message), record_time, message.position_state))
            elif topic == "/Mower/debug/lio":
                debug_lio.append((stamp(message), record_time,
                                  message.position_x, message.position_y))
            elif topic == "/Mower/localization_status":
                status.append((stamp(message), record_time, message.phase,
                               message.position_source, message.position_state,
                               message.position_valid, message.stop_required,
                               message.fault_code, message.reason))
            elif topic == "/Mower/localization_algorithm_status":
                alg_status.append((stamp(message), record_time, message.phase,
                                   message.position_source, message.position_state,
                                   message.position_valid, message.stop_required,
                                   message.fault_code, message.reason))
            elif topic.endswith("stop_car"):
                stops[topic].append((record_time, message.data))

    transitions = collections.Counter()
    runs = []
    if positions:
        run_state = positions[0][2]
        run_start = 0
        for index in range(1, len(positions)):
            state = positions[index][2]
            if state != run_state:
                transitions[f"{run_state}->{state}"] += 1
                runs.append((run_state, index - run_start))
                run_state, run_start = state, index
        runs.append((run_state, len(positions) - run_start))

    jumps = []
    for before, after in zip(positions, positions[1:]):
        delta = math.hypot(after[3] - before[3], after[4] - before[4])
        dt = after[0] - before[0]
        if delta > 0.25:
            jumps.append((round(before[0], 3), round(after[0], 3),
                          before[2], after[2], round(delta, 3), round(dt, 3)))
    jumps.sort(key=lambda event: event[4], reverse=True)

    # The debug topics are published after the production pose in one fusion
    # callback. This is a bag-order proxy, not an exact internal decision log.
    debug_gps_records = [x[1] for x in debug_gps]
    gps_by_stamp = sorted(gps, key=lambda sample: sample[0])
    gps_stamps = [x[0] for x in gps_by_stamp]

    def raw_for_stamp(t):
        raw_index = bisect.bisect_left(gps_stamps, t)
        raw_options = gps_by_stamp[max(raw_index - 1, 0):min(raw_index + 2, len(gps))]
        raw_match = min(raw_options, key=lambda x: abs(x[0] - t)) if raw_options else None
        return raw_match if raw_match and abs(raw_match[0] - t) < 0.001 else None

    def quality_passes(sample):
        return (sample is not None and sample[2] == 4 and sample[3] == 3 and
                sample[4] == 56 and sample[5] >= 8 and
                0 <= sample[6] <= 3 and 0 < sample[7] <= 0.30 and
                0 < sample[8] <= 0.30 and 0 < sample[9] <= 0.50)

    # Approximate the current GPS timestamp/speed gate on the *historically
    # selected* GPS stream. Equirectangular differences are sufficient for
    # local displacement; this deliberately omits heading/innovation/SE(2).
    current_basic_motion = []
    baseline = None
    for selected in debug_gps:
        if selected[0] == 0.0:
            current_basic_motion.append("unselected")
            continue
        raw_match = raw_for_stamp(selected[0])
        if not quality_passes(raw_match):
            current_basic_motion.append("quality_fail_or_raw_missing")
            continue
        t, lat, lon = raw_match[0], raw_match[10], raw_match[11]
        if baseline is None:
            baseline = (t, lat, lon)
            current_basic_motion.append("baseline")
            continue
        dt = t - baseline[0]
        if dt <= 0:
            current_basic_motion.append("time_reverse_or_duplicate")
            continue
        if not 0.01 <= dt <= 0.45:
            baseline = (t, lat, lon)
            current_basic_motion.append("baseline")
            continue
        dy = math.radians(lat - baseline[1]) * 6378137.0
        dx = math.radians(lon - baseline[2]) * 6378137.0 * math.cos(math.radians(lat))
        if math.hypot(dx, dy) > 2.0 * dt + 0.08:
            current_basic_motion.append("excessive_speed")
            continue
        baseline = (t, lat, lon)
        current_basic_motion.append("basic_motion_pass")

    def multiply(a, b):
        aw, ax, ay, az = a
        bw, bx, by, bz = b
        return (aw*bw-ax*bx-ay*by-az*bz,
                aw*bx+ax*bw+ay*bz-az*by,
                aw*by-ax*bz+ay*bw+az*bx,
                aw*bz+ax*by-ay*bx+az*bw)

    def yaw_of(q):
        w, x, y, z = q
        return math.atan2(2.0*(w*z+x*y), 1.0-2.0*(y*y+z*z))

    align = (math.cos(math.pi/4), 0.0, 0.0, math.sin(math.pi/4))
    pitch = (math.cos(math.radians(-12)/2), 0.0,
             math.sin(math.radians(-12)/2), 0.0)
    lio_yaw = [(x[0], x[1], yaw_of(multiply(multiply(align, x[3:7]), pitch)))
               for x in lio if x[2]]
    lio_yaw_records = [x[1] for x in lio_yaw]

    # Replay only the current straight/reverse/turn direction gate over the
    # old selected-sample stream. It is intentionally not a complete fusion
    # replay and does not model the innovation gate.
    axis_samples = []
    mismatch_count = 0
    last_mismatch = None
    axis_results = collections.Counter()
    current_observations = []
    previous_lio_yaw = None
    motion_baseline = None
    for frame in debug_gps:
        li = bisect.bisect_right(lio_yaw_records, frame[1]) - 1
        current_lio_yaw = lio_yaw[li] if li >= 0 else None
        turn_rate = None
        if current_lio_yaw and previous_lio_yaw:
            dt_lio = current_lio_yaw[0] - previous_lio_yaw[0]
            if 0 < dt_lio <= 0.60:
                dyaw = math.atan2(math.sin(current_lio_yaw[2]-previous_lio_yaw[2]),
                                  math.cos(current_lio_yaw[2]-previous_lio_yaw[2]))
                turn_rate = abs(dyaw) / dt_lio
        if current_lio_yaw:
            previous_lio_yaw = current_lio_yaw
        if turn_rate is not None and turn_rate >= 0.25:
            axis_samples = []
            mismatch_count = 0
            last_mismatch = None
        if frame[0] == 0.0:
            current_observations.append(("missing", current_lio_yaw[0] if current_lio_yaw else None))
            continue
        raw_match = raw_for_stamp(frame[0])
        if not quality_passes(raw_match):
            axis_samples = []
            mismatch_count = 0
            last_mismatch = None
            current_observations.append(("rejected", current_lio_yaw[0] if current_lio_yaw else None))
            continue
        t, lat, lon = raw_match[0], raw_match[10], raw_match[11]
        if motion_baseline is None:
            motion_baseline = (t, lat, lon)
            current_observations.append(("baseline", current_lio_yaw[0] if current_lio_yaw else None))
            continue
        dt = t - motion_baseline[0]
        if not 0.01 <= dt <= 0.45:
            if dt > 0:
                motion_baseline = (t, lat, lon)
            current_observations.append(("baseline" if dt > 0 else "rejected",
                                         current_lio_yaw[0] if current_lio_yaw else None))
            continue
        y = math.radians(lat) * 6378137.0
        x = math.radians(lon) * 6378137.0 * math.cos(math.radians(lat))
        old_y = math.radians(motion_baseline[1]) * 6378137.0
        old_x = (math.radians(motion_baseline[2]) * 6378137.0 *
                 math.cos(math.radians(motion_baseline[1])))
        if math.hypot(x-old_x, y-old_y) > 2.0*dt+0.08:
            current_observations.append(("rejected", current_lio_yaw[0] if current_lio_yaw else None))
            continue
        motion_baseline = (t, lat, lon)
        heading_trusted = raw_match[13] == 1 and 0 < raw_match[14] <= 5.0
        if not heading_trusted or turn_rate is None:
            axis_samples = []
            mismatch_count = 0
            last_mismatch = None
            current_observations.append(("accepted", current_lio_yaw[0] if current_lio_yaw else None))
            continue
        if turn_rate >= 0.25:
            axis_samples = [(t, x, y)]
            axis_results["turning"] += 1
            current_observations.append(("accepted", current_lio_yaw[0] if current_lio_yaw else None))
            continue
        if axis_samples and (t <= axis_samples[-1][0] or t-axis_samples[-1][0] > 0.65):
            axis_samples = []
            mismatch_count = 0
            last_mismatch = None
        axis_samples.append((t, x, y))
        axis_samples = [v for v in axis_samples if t-v[0] <= 0.65]
        if t-axis_samples[0][0] < 0.30:
            axis_results["insufficient"] += 1
            current_observations.append(("accepted", current_lio_yaw[0] if current_lio_yaw else None))
            continue
        dx, dy = x-axis_samples[0][1], y-axis_samples[0][2]
        if math.hypot(dx, dy) < 0.25:
            mismatch_count = 0
            last_mismatch = None
            axis_results["insufficient"] += 1
            current_observations.append(("accepted", current_lio_yaw[0] if current_lio_yaw else None))
            continue
        vehicle_yaw = math.atan2(math.sin(math.radians(raw_match[12]/100.0)),
                                 math.cos(math.radians(raw_match[12]/100.0)))
        difference = abs(math.atan2(math.sin(math.atan2(dy, dx)-vehicle_yaw),
                                    math.cos(math.atan2(dy, dx)-vehicle_yaw)))
        axis_error = min(difference, math.pi-difference)
        if axis_error > math.radians(40):
            if last_mismatch is None or math.hypot(x-last_mismatch[0], y-last_mismatch[1]) >= 0.25/3:
                mismatch_count += 1
                last_mismatch = (x, y)
            axis_results["rejected" if mismatch_count >= 3 else "suspect"] += 1
            current_observations.append(("heading_rejected" if mismatch_count >= 3 else "heading_suspect",
                                         current_lio_yaw[0] if current_lio_yaw else None))
        else:
            mismatch_count = 0
            last_mismatch = None
            axis_results["forward" if difference <= math.pi/2 else "reverse"] += 1
            current_observations.append(("accepted", current_lio_yaw[0] if current_lio_yaw else None))

    failed = False
    stable = 50
    missing_count = 0
    anchor_stamp = None
    predicted_transitions = collections.Counter()
    predicted_failure_causes = collections.Counter()
    for observation, lio_stamp in current_observations:
        was_failed = failed
        if observation == "accepted":
            missing_count = 0
            if failed:
                stable = min(stable + 1, 50)
                failed = stable < 50
            if not failed:
                anchor_stamp = lio_stamp
        elif observation in ("missing", "baseline", "rejected", "heading_rejected"):
            missing_count += 1
            if not failed and missing_count > 3:
                failed, stable = True, 28
            if (not failed and anchor_stamp is not None and lio_stamp is not None and
                    lio_stamp-anchor_stamp >
                    (1.50 if observation == "heading_rejected" else 0.75)):
                failed, stable = True, 28
        elif observation == "heading_suspect":
            missing_count = 0
            if (not failed and anchor_stamp is not None and lio_stamp is not None and
                    lio_stamp-anchor_stamp > 1.50):
                failed, stable = True, 28
        else:
            missing_count = 0
            failed, stable = True, 28
        if failed != was_failed:
            predicted_transitions["healthy_to_fallback" if failed else "fallback_to_healthy"] += 1
            if failed:
                predicted_failure_causes[observation] += 1
    switch_causes = collections.Counter()
    rejected_switch_quality = collections.Counter()
    rejected_switch_current_motion = collections.Counter()
    rejected_switch_selected_dts = []
    rejected_switch_examples = []
    lio_debug_by_record = sorted(debug_lio, key=lambda sample: sample[1])
    lio_debug_records = [x[1] for x in lio_debug_by_record]

    def same_callback_debug_lio(record_time):
        index = bisect.bisect_left(lio_debug_records, record_time)
        if index >= len(lio_debug_by_record):
            return None
        result = lio_debug_by_record[index]
        return result if result[1] - record_time <= 0.05 else None

    switch_corrections = []
    for before, after in zip(positions, positions[1:]):
        if before[2] == after[2] or before[2] not in (1, 2, 5) or after[2] not in (1, 2, 5):
            continue
        index = bisect.bisect_left(debug_gps_records, after[1])
        if index < len(debug_gps) and debug_gps[index][1] - after[1] <= 0.05:
            debug = debug_gps[index]
            label = ("unselected" if debug[0] == 0.0 else
                     "selected_rejected" if debug[2] == 0 else "selected_accepted")
        else:
            label = "no_debug_pair"
        switch_causes[f"{before[2]}->{after[2]}:{label}"] += 1
        if before[2] == 1 and label == "selected_rejected":
            raw_match = raw_for_stamp(debug[0])
            basic_good = quality_passes(raw_match)
            if raw_match:
                rejected_switch_quality["basic_quality_pass" if basic_good else "basic_quality_fail"] += 1
            else:
                rejected_switch_quality["raw_not_found"] += 1
            rejected_switch_current_motion[current_basic_motion[index]] += 1
            previous = index - 1
            while previous >= 0 and debug_gps[previous][0] == 0.0:
                previous -= 1
            selected_dt = debug[0] - debug_gps[previous][0] if previous >= 0 else None
            if selected_dt is not None:
                rejected_switch_selected_dts.append(selected_dt)
            if len(rejected_switch_examples) < 5:
                rejected_switch_examples.append((round(after[0], 3),
                                                 round(debug[0], 3),
                                                 round(selected_dt, 4) if selected_dt is not None else None,
                                                 bool(raw_match and basic_good)))
        old_lio = same_callback_debug_lio(before[1])
        new_lio = same_callback_debug_lio(after[1])
        if old_lio and new_lio:
            correction = math.hypot(after[3] - before[3] - (new_lio[2] - old_lio[2]),
                                    after[4] - before[4] - (new_lio[3] - old_lio[3]))
            switch_corrections.append((before[2], after[2], correction,
                                       before[0], after[0]))

    lio_dt = [after[0] - before[0] for before, after in zip(lio, lio[1:])
              if before[2] and after[2] and after[0] > before[0]]
    gps_dt = [after[0] - before[0] for before, after in zip(gps, gps[1:])
              if after[0] > before[0]]
    quality = sum(x[2] == 4 and x[3] == 3 and x[4] == 56 and x[5] >= 8
                  and 0 <= x[6] <= 3 and 0 < x[7] <= 0.30
                  and 0 < x[8] <= 0.30 and 0 < x[9] <= 0.50 for x in gps)

    # A bag recorder's arrival order is only a proxy for fusion's callback order.
    # Count debug frames with no chosen GPS that have a raw GPS stamp within
    # +/-50 ms, separately by whether that raw sample was recorded later.
    no_chosen = sum(x[0] == 0.0 for x in debug_gps)
    rejected_or_missing = sum(x[2] == 0 for x in debug_gps)
    nearby = later_than_lio_record = later_than_debug_record = 0
    lio_by_record = sorted(lio, key=lambda sample: sample[1])
    lio_records = [x[1] for x in lio_by_record]
    for frame in debug_gps:
        if frame[0] != 0.0:
            continue
        lio_index = bisect.bisect_right(lio_records, frame[1]) - 1
        if lio_index < 0 or frame[1] - lio_by_record[lio_index][1] > 0.25:
            continue
        lio_stamp, lio_record = lio_by_record[lio_index][:2]
        index = bisect.bisect_left(gps_stamps, lio_stamp)
        options = gps_by_stamp[max(index - 2, 0):min(index + 3, len(gps_by_stamp))]
        matches = [x for x in options if abs(x[0] - lio_stamp) <= 0.05]
        if matches:
            nearby += 1
            nearest = min(matches, key=lambda x: abs(x[0] - lio_stamp))
            if nearest[1] > lio_record:
                later_than_lio_record += 1
            if nearest[1] > frame[1]:
                later_than_debug_record += 1

    return {
        "bag": str(path),
        "position_count": len(positions),
        "position_states": dict(collections.Counter(x[2] for x in positions)),
        "position_transitions": dict(transitions),
        "source_switch_debug_proxy": dict(switch_causes),
        "rtk_to_lio_selected_rejected_basic_quality": dict(rejected_switch_quality),
        "rtk_to_lio_selected_rejected_current_basic_motion_proxy": dict(rejected_switch_current_motion),
        "rtk_to_lio_selected_rejected_dt_p50_p95_s": [
            percentile(rejected_switch_selected_dts, p) for p in (0.5, 0.95)],
        "rtk_to_lio_selected_rejected_examples": rejected_switch_examples,
        "current_direction_gate_old_stream_proxy": dict(axis_results),
        "current_outage_policy_old_stream_proxy": dict(predicted_transitions),
        "current_outage_policy_failure_causes_proxy": dict(predicted_failure_causes),
        "source_switch_correction_pairs": len(switch_corrections),
        "source_switch_correction_gt_025_m": sum(x[2] > 0.25 for x in switch_corrections),
        "source_switch_correction_p50_p95_max_m": [
            percentile([x[2] for x in switch_corrections], p) for p in (0.5, 0.95, 1.0)],
        "largest_source_switch_corrections": [
            (x[0], x[1], round(x[2], 3), round(x[3], 3), round(x[4], 3))
            for x in sorted(switch_corrections, key=lambda x: x[2], reverse=True)[:6]],
        "state_run_lengths": {str(state): {
            "count": len([length for s, length in runs if s == state]),
            "median": statistics.median([length for s, length in runs if s == state])
        } for state in sorted({s for s, _ in runs})},
        "largest_xy_jumps_gt_025_m": jumps[:12],
        "jump_gt_025_m_count": len(jumps),
        "lio_count": len(lio),
        "lio_dt_p50_p95_p99_s": [percentile(lio_dt, p) for p in (0.5, 0.95, 0.99)],
        "lio_dt_gt_060_count": sum(x > 0.60 for x in lio_dt),
        "gps_count": len(gps),
        "gps_dt_p50_p95_p99_s": [percentile(gps_dt, p) for p in (0.5, 0.95, 0.99)],
        "gps_raw_basic_quality_pass_count": quality,
        "debug_gps_no_chosen_count": no_chosen,
        "debug_gps_rejected_or_missing_count": rejected_or_missing,
        "no_chosen_raw_gps_within_50ms": nearby,
        "no_chosen_nearest_raw_gps_recorded_after_lio": later_than_lio_record,
        "no_chosen_nearest_raw_gps_recorded_after_debug": later_than_debug_record,
        "manager_status_count": len(status),
        "algorithm_status_count": len(alg_status),
        "manager_status_phase_counts": dict(collections.Counter(x[2] for x in status)),
        "algorithm_status_phase_counts": dict(collections.Counter(x[2] for x in alg_status)),
        "manager_fault_valid_stop_count": sum(bool(x[5] and x[6] and x[7]) for x in status),
        "algorithm_fault_valid_stop_count": sum(bool(x[5] and x[6] and x[7]) for x in alg_status),
        "stop_messages": {k: dict(collections.Counter(str(x[1]) for x in v))
                          for k, v in stops.items()},
    }


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("bags", nargs="+", type=Path)
    args = parser.parse_args()
    for bag in args.bags:
        print(json.dumps(summarize(bag), ensure_ascii=False))
