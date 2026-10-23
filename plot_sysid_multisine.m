%% 复现 sysid_recorder 的 multisine 激励波形
% 对应配置 JSON:
%   drive: {"type":"multisine","offset":1500,"A2":500,"A4":200}
%   turn : {"type":"multisine","offset":1500,"A1":300,"A3":250,"A5":150}
% 生成逻辑与 crawler_control/src/sysid_recorder/src/sysid_recorder_node.cpp
% 中 WaveformConfig::multisineValue / zeroPeakHold 完全一致。
%   s = A1 sin(wt) + A3 sin(3wt) + A5 sin(5wt)      <- 转向通道（奇次谐波）
%     + A2 sin(2wt+P2) + A4 sin(4wt+P4)             <- 前进通道（偶次谐波）
% 再经过零保持 / 峰值保持 / 零阶保持(ZOH)，输出 = offset + 上面的 s。
clear; clc; close all

%% ---- 未在 JSON 中给出的参数：用 C++ 代码里的默认值 ----
T0    = 60;       % 周期 s
fc    = 50;       % 零阶保持频率 Hz（时间先量化到 1/fc 网格再算波形）
pZero = 0.10;     % 过零保持比例
pPeak = 0.12;     % 极值保持比例
P2    = 4.8860;   % 2次谐波相位 rad（离线寻优，使轨迹近似闭合、外接框方正）
P4    = 4.5729;   % 4次谐波相位 rad

%% ---- 用户 JSON 配置 ----
drive.offset = 1500;
drive.A2 = 200;  drive.A4 = 100;              % 前进通道：偶次谐波
turn.offset  = 1500;
turn.A1 = 120;  turn.A3 = 110;  turn.A5 = 100; % 转向通道：奇次谐波

%% ---- 时间轴：按节点定时器 20Hz 采样，画 2 个周期 ----
Ts = 1/20;
t = 0:Ts:2*T0;

u_drive_raw = multisineValue(t, T0, fc, pZero, pPeak, [0 0 0], [drive.A2 drive.A4], [P2 P4]);
u_turn_raw  = multisineValue(t, T0, fc, pZero, pPeak, [turn.A1 turn.A3 turn.A5], [0 0], [0 0]);
u_drive = drive.offset + u_drive_raw;
u_turn  = turn.offset  + u_turn_raw;

%% ---- 图 1：两通道时域波形（阶梯线表示 50Hz 零阶保持的实际输出）----
figure('Name','multisine 时域波形','Position',[80 80 1100 720]);

subplot(2,1,1)
stairs(t, u_drive, 'LineWidth', 1.2); grid on
hold on
plot(t, drive.offset + u_drive_raw, '--', 'Color', [0.7 0.7 0.7]) % 保持处理前的原始多频正弦
yline(drive.offset, ':')
xlabel('t (s)'); ylabel('drive\_cmd')
title(sprintf('前进通道: offset=%g + A2 sin(2wt+%.3f) + A4 sin(4wt+%.3f)', ...
      drive.offset, P2, P4))
legend('实际输出(ZOH+保持)','保持处理前','offset','Location','best')

subplot(2,1,2)
stairs(t, u_turn, 'LineWidth', 1.2); grid on
hold on
plot(t, turn.offset + u_turn_raw, '--', 'Color', [0.7 0.7 0.7])
yline(turn.offset, ':')
xlabel('t (s)'); ylabel('turn\_cmd')
title(sprintf('转向通道: offset=%g + A1 sin(wt) + A3 sin(3wt) + A5 sin(5wt)', turn.offset))
legend('实际输出(ZOH+保持)','保持处理前','offset','Location','best')

%% ---- 图 2：前进-转向 指令相平面轨迹（奇偶分频使两通道频谱不重叠）----
figure('Name','drive-turn 相平面','Position',[220 160 640 560]);
plot(u_drive, u_turn, 'LineWidth', 1.0); grid on
hold on
plot(u_drive(1), u_turn(1), 'go', 'MarkerFaceColor','g')   % 起点
xlabel('drive\_cmd'); ylabel('turn\_cmd')
title('drive-turn 指令轨迹（2 个周期，绿点为起点）')
axis equal

%% ---------------- 局部函数：与 C++ 实现一一对应 ----------------
function s = multisineValue(t, T0, fc, pZero, pPeak, A_odd, A_even, P_even)
% 多频正弦 + 双边保持(zeroPeakHold) + 零阶保持(ZOH)
% A_odd = [A1 A3 A5]，A_even = [A2 A4]，P_even = [P2 P4]
if T0 <= 0, T0 = 60; end
if fc  <= 0, fc  = 50; end
tq = floor(t*fc + 1e-9)/fc;          % ZOH 时间量化（1e-9 容错同 C++）
w  = 2*pi/T0;
s  = zeros(size(tq));
for k = 1:numel(A_odd)                % 奇次谐波，相位恒 0
    if A_odd(k) ~= 0
        s = s + A_odd(k)*sin((2*k-1)*w*tq);
    end
end
for k = 1:numel(A_even)               % 偶次谐波，带优化相位
    if A_even(k) ~= 0
        s = s + A_even(k)*sin(2*k*w*tq + P_even(k));
    end
end
Asum = sum(abs(A_odd)) + sum(abs(A_even));
s = zeroPeakHold(s, Asum, pZero, pPeak);
end

function y = zeroPeakHold(s, A, p_zero, p_peak)
% 双边保持：过零贴零 + 峰值贴平（与 C++ zeroPeakHold 一致）
if p_peak >= 0.95, p_peak = 0.95; end
if p_zero > 0
    s_z = A*sin(pi*p_zero/2);
else
    s_z = 0;
end
A_h = A*cos(pi*p_peak/2);
y = s;
y(s >=  A_h)    =  A_h;   % 正峰值段 -> 平顶
y(s <= -A_h)    = -A_h;   % 负峰值段 -> 平顶
y(abs(s) < s_z) = 0;      % 过零段   -> 0
end
