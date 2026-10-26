% apply_correction.m
%
% The hand derivation in the engineering notebook back-solved the f anchor
% from the three "stand on test foot only" trials and got a correction
% factor k = 0.7269. This script does the parts not done by hand:
%
%   1. fits a line through all 21 standing trials (true % vs displayed %)
%      as an independent check on k
%   2. applies k to all 21 and re-bands them
%   3. scores the device as an over/under-the-limit alarm, which is how it
%      actually behaves for a patient
%   4. draws the parity plot
%
% Data embedded so it runs with no setup.

clear; clc; close all;

k = 0.7269;                       % back-solved by hand
BANDS = [10 30 50 85];
NAMES = {'<10%','10-30%','30-50%','50-85%','>85%'};

cond = {'featherweight','featherweight','featherweight', ...
        'flat foot press','flat foot press','flat foot press', ...
        '30 percent','30 percent','30 percent', ...
        '50 percent','50 percent','50 percent', ...
        'stand on both feet','stand on both feet','stand on both feet', ...
        '85 percent','85 percent','85 percent', ...
        'stand on test foot only','stand on test foot only','stand on test foot only'};
tr = [1 2 3 1 2 3 1 2 3 1 2 3 1 2 3 1 2 3 1 2 3];

truePct = [ 8.91 10.87 14.90 22.68 23.18 22.29 29.82 27.64 27.79 ...
           50.22 49.49 49.60 49.53 49.55 49.57 76.58 79.11 77.88 ...
           98.33 97.29 97.64];

shownPct = [16.40 20.20 26.40 32.80 30.50 28.50 48.00 37.35 38.10 ...
            73.40 69.00 71.10 56.85 61.20 63.45 108.50 112.10 112.80 ...
            125.00 138.80 139.30];

% ---------- 1. independent check: fit true = m * shown ----------
% forced through the origin, since 0% must map to 0%
m   = sum(shownPct .* truePct) / sum(shownPct.^2);
fit = m * shownPct;
R2  = 1 - sum((truePct - fit).^2) / sum((truePct - mean(truePct)).^2);

fprintf('=== CHECK ===\n');
fprintf('fitted slope (true vs shown) : %.4f   R2 = %.4f\n', m, R2);
fprintf('back-solved k                : %.4f\n', k);
fprintf('difference                   : %.1f%%\n\n', 100*abs(m-k)/k);

% ---------- 2. apply k and re-band ----------
corr = shownPct * k;
bandOf = @(p) 1 + sum(p >= BANDS);
nOK = 0; nLow = 0;

fprintf('=== APPLY k = %.4f ===\n', k);
fprintf('%-24s %3s %8s %8s %8s %11s %11s %6s\n', ...
        'condition','tr','true','shown','corr','true band','corr band','match');
for i = 1:numel(truePct)
    tb = bandOf(truePct(i));
    cb = bandOf(corr(i));
    ok = (tb == cb);
    nOK  = nOK + ok;
    nLow = nLow + (cb < tb);
    fprintf('%-24s %3d %8.1f %8.1f %8.1f %11s %11s %6s\n', ...
            cond{i}, tr(i), truePct(i), shownPct(i), corr(i), ...
            NAMES{tb}, NAMES{cb}, string(ok));
end

err = abs(corr - truePct);
fprintf('\nbands correct    : %d/21\n', nOK);
fprintf('mean abs error   : %.2f percentage points\n', mean(err));
fprintf('max abs error    : %.2f percentage points\n', max(err));
fprintf('bands below truth: %d\n\n', nLow);

% ---------- 3. score it as an over/under alarm ----------
fprintf('=== AS A CEILING ALARM ===\n');
fprintf('%6s %10s %22s %14s\n','limit','correct','missed overload','false alarm');
for L = [10 15 20 25 30 50]
    tOver = truePct > L;
    dOver = corr     > L;
    fprintf('%5d%% %8d/21 %18d %14d\n', L, sum(tOver==dOver), ...
            sum(tOver & ~dOver), sum(~tOver & dOver));
end

% ---------- 4. parity plot ----------
figure('Color','w','Position',[80 80 620 580]);
lim = [0 150];
plot(lim, lim, 'k--', 'LineWidth', 1.2); hold on
plot(lim, m*lim, 'b-', 'LineWidth', 1.6);
plot(shownPct, truePct, 'ko', 'MarkerFaceColor', [.25 .45 .8], 'MarkerSize', 7);
grid on; box on; axis equal; xlim(lim); ylim([0 110]);
xlabel('my device displayed (%BW)', 'FontWeight','bold');
ylabel('true load, force plate (%BW)', 'FontWeight','bold');
title({'Device vs force plate, 21 standing trials', ...
       sprintf('slope %.4f, R^2 = %.4f  (perfect device would be slope 1)', m, R2)});
legend({'perfect device (y = x)', sprintf('best fit (y = %.4fx)', m), 'trials'}, ...
       'Location','southeast');
exportgraphics(gcf, 'parity_plot.png', 'Resolution', 300);
fprintf('\nSaved parity_plot.png\n');
