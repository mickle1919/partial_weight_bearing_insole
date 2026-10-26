% validate_median_fix.m
%
% Held-out validation of the corrected f capture.
%
% The firmware sets the 100% anchor by taking the MEDIAN of a 4-second hold
% (CAL_N = 200 samples at delay(20)). This script reproduces that from the
% device's OWN logged forces during the three "stand on test foot only"
% holds, using no force-plate data at all, and then scores the remaining 18
% standing trials against force-plate truth.
%
% The 3 anchor holds are excluded from scoring, so the 18 are held out.
%
% Baseline 18.02 lb, from the two-anchor solve in the notebook. No /0.98
% term: one-foot stance measured ~98% BW on the plate, and that is already
% accounted for in the truth values rather than in the anchor.
%
% The firmware calibrates from ONE hold, so each of the three is scored
% separately rather than averaged.

clear; clc;
warning('off','MATLAB:table:ModifiedAndSavedVarnames');

% ---------------- SET THIS ----------------
ROOT = '/Users/michaelphelan/Desktop/LAB DATA';
% ------------------------------------------

BASE   = 18.02;    % stored baseline, lb
WIN    = 4.0;      % seconds, matching CAL_N = 200 @ delay(20)
START  = 1.0;      % skip the first second, as the firmware does
TARGET = 59.46;    % back-solved correct anchor
ANCHOR = 'stand on test foot only';

conds = {'featherweight','flat foot press','30 percent','50 percent', ...
         'stand on both feet','85 percent','stand on test foot only'};

% force-plate truth, %BW, in trial order
truth = { [ 8.91 10.87 14.90]
          [22.68 23.18 22.29]
          [29.82 27.64 27.79]
          [50.22 49.49 49.60]
          [49.53 49.55 49.57]
          [76.58 79.11 77.88]
          [98.33 97.29 97.64] };

edges  = [10 30 50 85];
names  = {'<10%','10-30%','30-50%','50-85%','>85%'};
bandOf = @(p) 1 + sum(p >= edges);

% ---------- per-trial 4 s median of total logged force ----------
medTotal = cell(size(conds));
for ci = 1:numel(conds)
    cdir = fullfile(ROOT, conds{ci});
    dv = dir(fullfile(cdir,'my_data_*.csv'));
    if isempty(dv), error('no my_data_*.csv in %s', cdir); end
    T = readtable(fullfile(cdir, dv(1).name));

    vn = T.Properties.VariableNames;
    tc = vn{find(contains(lower(vn),'time'),1)};
    rc = vn{find(contains(lower(vn),'trial'),1)};
    hc = vn{find(contains(lower(vn),'heel') & contains(lower(vn),'lb'),1)};
    bc = vn{find(contains(lower(vn),'ball') & contains(lower(vn),'lb'),1)};

    ids = unique(T.(rc)); ids = ids(max(1,end-2):end);   % last 3 trials
    v = nan(1,numel(ids));
    for k = 1:numel(ids)
        sel = T.(rc) == ids(k);
        t = T.(tc)(sel); if max(t) > 100, t = t/1000; end
        t = t - t(1);
        s = T.(hc)(sel) + T.(bc)(sel);
        m = t >= START & t <= min(START + WIN, max(t));
        if nnz(m) < 5, m = true(size(t)); end
        v(k) = median(s(m));
    end
    medTotal{ci} = v;
end

% ---------- STEP 1: the anchors, from the device alone ----------
ai = find(strcmp(conds, ANCHOR));
a  = medTotal{ai};

fprintf('=== STEP 1: anchor from the device alone, no plate data ===\n');
for k = 1:numel(a)
    fprintf('  hold %d: 4 s median = %7.2f lb  (%+.1f%% vs %.2f)\n', ...
            k, a(k), 100*(a(k)-TARGET)/TARGET, TARGET);
end
fprintf('  spread across holds: %.2f lb (%.1f%%)\n', ...
        max(a)-min(a), 100*(max(a)-min(a))/median(a));
fprintf('  the MEAN this replaced stored 48.14 lb (%.1f%% low)\n\n', ...
        100*(48.14-TARGET)/TARGET);

% ---------- STEP 2: score the 18 held-out trials, per hold ----------
fprintf('=== STEP 2: 18 held-out trials, scored under each hold''s anchor ===\n');
summary = nan(numel(a), 6);
rows = {};

for h = 1:numel(a)
    F = a(h);
    nOK = 0; nLow = 0; errs = [];
    fprintf('\n--- anchor from hold %d (%.2f lb) ---\n', h, F);
    fprintf('%-22s %3s %8s %8s %10s %10s %7s\n', ...
            'condition','tr','true %','new %','true band','new band','match');
    for ci = 1:numel(conds)
        if ci == ai, continue, end                 % held out
        v = medTotal{ci}; tr = truth{ci};
        for k = 1:min(numel(v), numel(tr))
            p  = (v(k) - BASE) / (F - BASE) * 100;
            tb = bandOf(tr(k));
            nb = bandOf(p);
            ok = (tb == nb);
            nOK = nOK + ok; nLow = nLow + (nb < tb);
            errs(end+1) = abs(p - tr(k)); %#ok<SAGROW>
            fprintf('%-22s %3d %8.1f %8.1f %10s %10s %7s\n', ...
                    conds{ci}, k, tr(k), p, names{tb}, names{nb}, string(ok));
            rows(end+1,:) = {h, F, conds{ci}, k, tr(k), p, abs(p-tr(k)), ...
                             names{tb}, names{nb}, ok}; %#ok<SAGROW>
        end
    end
    fprintf('  bands %d/%d | mean abs err %.2f pts | max %.2f pts | banded LOW %d\n', ...
            nOK, numel(errs), mean(errs), max(errs), nLow);
    summary(h,:) = [F nOK mean(errs) max(errs) nLow 100*(F-TARGET)/TARGET];
end

% ---------- STEP 3: summary ----------
fprintf('\n=== SUMMARY: one hold is what the firmware actually uses ===\n');
fprintf('%6s %10s %10s %12s %10s %12s\n', ...
        'hold','anchor','bands','mean err','max err','banded LOW');
for h = 1:size(summary,1)
    fprintf('%6d %9.2f %8d/18 %10.2f %10.2f %10d\n', ...
            h, summary(h,1), summary(h,2), summary(h,3), summary(h,4), summary(h,5));
end
fprintf('\nrange across holds: %d to %d of 18 bands correct\n', ...
        min(summary(:,2)), max(summary(:,2)));
fprintf('\nAnchor used no force-plate data. The 3 %s holds are excluded.\n', ANCHOR);

% ---------- STEP 4: write the csvs ----------
Tt = cell2table(rows, 'VariableNames', {'hold','anchor_lb','condition','trial', ...
     'true_pct_BW','device_pct','abs_error_pts','true_band','device_band','band_match'});
writetable(Tt, 'median_fix_trials.csv');

Ts = table((1:size(summary,1))', summary(:,1), summary(:,6), summary(:,2), ...
           repmat(18,size(summary,1),1), summary(:,3), summary(:,4), summary(:,5), ...
     'VariableNames', {'hold','anchor_lb','anchor_error_pct','bands_correct', ...
                       'trials','mean_abs_error_pts','max_abs_error_pts','banded_below_truth'});
writetable(Ts, 'median_fix_summary.csv');

fprintf('\nSaved median_fix_trials.csv and median_fix_summary.csv\n');
