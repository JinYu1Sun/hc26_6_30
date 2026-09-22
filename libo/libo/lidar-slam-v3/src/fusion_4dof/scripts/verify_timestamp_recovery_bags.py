#!/usr/bin/env python3
"""Deterministic regression for the September timestamp-backlog incident.

This reads existing ROS1 bags without starting ROS.  Record time is used as
the replay "now", matching the freshness decisions in fusion_4dof and
android_manager_4dof.  It validates both rejection of the recorded fault and
acceptance of the healthy recordings; it is not a replacement for an on-robot
ROS1 integration replay.
"""

import argparse
import struct
from dataclasses import dataclass
from pathlib import Path

from rosbags.highlevel import AnyReader


@dataclass
class Stream:
    samples: list
    regressions: int = 0

    @property
    def fresh_count(self):
        return sum(sample[2] for sample in self.samples)

    @property
    def ages(self):
        return [sample[1] for sample in self.samples]


def stamp(message):
    value = message.header.stamp
    return value.sec + value.nanosec * 1e-9


def is_fresh(value, record_time, max_age, max_future):
    age = record_time - value
    return value > 0.0 and -max_future <= age <= max_age


def longest_bad_run(stream, ignore_before_first_fresh=False):
    longest = 0.0
    start = None
    armed = not ignore_before_first_fresh
    for record_time, _, fresh in stream.samples:
        if fresh:
            armed = True
            start = None
        elif armed:
            if start is None:
                start = record_time
            longest = max(longest, record_time - start)
    return longest


def read_bag(path, max_age, max_future):
    streams = {}
    lidar_rollbacks = 0
    scan_end = Stream([])
    with AnyReader([path]) as reader:
        connections = [
            connection for connection in reader.connections
            if connection.topic in {
                "/rosout", "/Mower/lio_slam", "/Mower/position",
                "/livox/imu", "/livox/points"
            }
        ]
        previous = {}
        for connection, record_ns, raw in reader.messages(
                connections=connections):
            message = reader.deserialize(raw, connection.msgtype)
            record_time = record_ns * 1e-9
            if connection.topic == "/rosout":
                if (connection.ext.callerid == "/laserMapping" and
                        "lidar loop back" in message.msg):
                    lidar_rollbacks += 1
                continue
            caller = connection.ext.callerid
            key = (connection.topic, caller)
            if connection.topic == "/livox/points":
                field = next(item for item in message.fields if item.name == "t")
                count = message.width * message.height
                data = memoryview(message.data)
                max_offset_ns = max(
                    struct.unpack_from(
                        "<I", data, index * message.point_step + field.offset)[0]
                    for index in range(count)) if count else 0
                value = stamp(message) + max_offset_ns * 1e-9
                target = scan_end
            else:
                value = stamp(message)
                target = streams.setdefault(key, Stream([]))
            if key in previous and value < previous[key]:
                target.regressions += 1
            previous[key] = value
            age = record_time - value
            target.samples.append(
                (record_time, age,
                 is_fresh(value, record_time, max_age, max_future)))
    return streams, scan_end, lidar_rollbacks


def require(condition, description):
    if not condition:
        raise AssertionError(description)
    print("PASS:", description)


def stream(streams, topic, caller):
    result = streams.get((topic, caller))
    require(result is not None and result.samples,
            f"{topic} from {caller} exists")
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("bag_dir", nargs="?", type=Path, default=Path("."))
    parser.add_argument("--max-age", type=float, default=0.50)
    parser.add_argument("--max-future", type=float, default=0.05)
    args = parser.parse_args()

    names = {
        "baseline": "0921chushihua.bag",
        "incident": "2026-09-21-11-48-18.bag",
        "stuck": "2026-09-21-12-23-14.bag",
        "recovered": "2026-09-21-12-27-30.bag",
        "raw": "lidar.bag",
    }
    reports = {}
    for role, name in names.items():
        path = args.bag_dir / name
        require(path.is_file(), f"{name} exists")
        reports[role] = read_bag(path, args.max_age, args.max_future)

    baseline = stream(reports["baseline"][0], "/Mower/lio_slam",
                      "/laserMapping")
    require(baseline.fresh_count == len(baseline.samples),
            "baseline LIO is entirely fresh")
    require(baseline.regressions == 0, "baseline LIO timestamps are monotonic")

    incident = stream(reports["incident"][0], "/Mower/lio_slam",
                      "/laserMapping")
    require(reports["incident"][2] >= 6,
            "incident contains the recorded LiDAR rollback events")
    require(longest_bad_run(incident) > 10.0,
            "incident contains a stale-LIO run beyond restart timeout")

    stuck = stream(reports["stuck"][0], "/Mower/lio_slam",
                   "/laserMapping")
    require(stuck.fresh_count == 0,
            "all LIO frames in the stuck bag fail the current freshness gate")
    require(longest_bad_run(stuck) > 10.0,
            "stuck LIO cannot be misclassified as healthy by callback rate")

    recovered_streams = reports["recovered"][0]
    recovered_lio = stream(recovered_streams, "/Mower/lio_slam",
                           "/laserMapping")
    require(recovered_lio.fresh_count / len(recovered_lio.samples) > 0.99,
            "more than 99% of recovered LIO frames pass")
    require(longest_bad_run(recovered_lio, True) < 1.0,
            "post-start recovered LIO has no one-second stale run")
    fusion_position = stream(recovered_streams, "/Mower/position",
                             "/fusion_4dof")
    require(fusion_position.fresh_count == len(fusion_position.samples),
            "all recovered fusion positions pass")
    replay_position = stream(
        recovered_streams, "/Mower/position",
        "/play_1789965098273147385")
    require(replay_position.fresh_count == 0,
            "all foreign historical position frames are rejected")

    raw_streams, scan_end, _ = reports["raw"]
    raw_imu = stream(raw_streams, "/livox/imu",
                     "/livox_lidar_publisher2")
    require(raw_imu.fresh_count == len(raw_imu.samples),
            "all recorded raw IMU frames pass")
    require(raw_imu.regressions == 0, "raw IMU timestamps are monotonic")
    require(scan_end.samples and scan_end.fresh_count == len(scan_end.samples),
            "all PointCloud2 scan-end timestamps pass")
    require(scan_end.regressions == 0,
            "PointCloud2 scan-end timestamps are monotonic")

    print("RESULT: timestamp recovery bag regression PASS")


if __name__ == "__main__":
    main()
