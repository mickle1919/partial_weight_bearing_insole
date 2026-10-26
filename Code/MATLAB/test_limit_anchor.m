% test_limit_anchor.m
%
% Does the upper calibration anchor have to sit at full body weight?
%
% The firmware originally set it by standing on the test foot alone, which
% is the one thing a partial weight-bearing patient cannot do. The
% alternative is to anchor at the prescribed limit: the patient presses the
% foot on a bathroom scale until it reads their limit, then calibrates.
%
% 30% and 50% are the two limits actually prescribed, so those are the two
% alternative anchors tested here, against the current one.
%
% The span: if the device logs v_L pounds at a known load of L percent body
% weight, then
%
%       percent = (v - b) / (v_L - b) * L
%
% All three anchor conditions are excluded from scoring, so the same 12
% trials are scored under every method and the comparison is like for like.

clear; clc;
warning('off','MATLAB:table:ModifiedAndSavedVarnames');

% ---------------- SET THIS ----------------
ROOT = '/Users/michaelphelan/Desktop/LAB DATA';
% ------------------------------------------

BASE  = 18.02;    % stored baseline, lb
WIN   = 4.0;      % seconds, matching CAL_N = 200 @ delay(20)
START = 1.0;      % skip the first second, as the firmware does

ANCHORS = {'stand on test foot only','30 percent','50 percent'};
LABELS  = {'current (100%)','limit 30%','limit 50%'};

conds = {'featherweight','flat foot press','30 percent','50 percent', ...
         'stand on both feet','85 percent','stand on test foot only'};

truth = { [ 8.91 10.87 14.90]
          [22.68 23.18 22.29]
          [29.82 27.64 27.79]
          [50.22 49.49 49.60]
          [49.53 49.55 49.57]
          [76.58 79.11 77.88]
          [98.33 97.29 97.64] };

edges  = [10 30 50 85];
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

    ids = unique(T.(rc)); ids = ids(max(1,end-2):end);
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

ai = cellfun(@(x) find(strcmp(conds,x)), ANCHORS);

% ---------- STEP 1: the three anchors ----------
fprintf('=== STEP 1: the three anchors ===\n');
fprintf('%-16s %-24s %10s %12s\n','method','condition','load %BW','logged lb');
aV = nan(1,3); aL = nan(1,3);
for j = 1:3
    aV(j) = median(medTotal{ai(j)});
    aL(j) = median(truth{ai(j)});
    fprintf('%-16s %-24s %10.2f %12.2f\n', LABELS{j}, ANCHORS{j}, aL(j), aV(j));
end

% ---------- STEP 2: the common 12 trials ----------
keep = setdiff(1:numel(conds), ai);
tp = []; pp = nan(3,0);
for ci = keep
    v = medTotal{ci}; tr = truth{ci};
    for k = 1:min(numel(v), numel(tr))
        tp(end+1) = tr(k); %#ok<SAGROW>
        for j = 1:3
            pp(j, numel(tp)) = (v(k) - BASE)/(aV(j) - BASE) * aL(j);
        end
    end
end

fprintf('\n=== STEP 2: the 12 trials no anchor used ===\n');
fprintf('%-22s %3s %8s %12s %11s %11s\n', ...
        'condition','tr','true %','current','limit 30%','limit 50%');
n = 0;
for ci = keep
    v = medTotal{ci}; tr = truth{ci};
    for k = 1:min(numel(v), numel(tr))
        n = n + 1;
        fprintf('%-22s %3d %8.1f %12.1f %11.1f %11.1f\n', ...
                conds{ci}, k, tp(n), pp(1,n), pp(2,n), pp(3,n));
    end
end

% ---------- STEP 3: scores ----------
fprintf('\n=== STEP 3: scores ===\n');
fprintf('%-16s | %10s %9s %5s | %12s %9s %5s\n', ...
        'method','bands','mean err','low','bands <=30%','err <=30%','low');
sub = tp <= 30;
for j = 1:3
    ok  = arrayfun(@(i) bandOf(pp(j,i))==bandOf(tp(i)), 1:numel(tp));
    low = arrayfun(@(i) bandOf(pp(j,i))< bandOf(tp(i)), 1:numel(tp));
    err = abs(pp(j,:) - tp);
    fprintf('%-16s | %7d/%-2d %9.2f %5d | %9d/%-2d %9.2f %5d\n', ...
            LABELS{j}, sum(ok), numel(tp), mean(err), sum(low), ...
            sum(ok & sub), nnz(sub), mean(err(sub)), sum(low & sub));
end

fprintf('\n"low" counts bands below the truth, the unsafe direction.\n');
