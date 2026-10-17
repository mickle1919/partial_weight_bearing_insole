%% analyze_standing_trials.m
% Set one: 7 standing conditions, 3 trials each (21 trials).
% For each trial: read the force plate .forces file, average the steady
% hold, convert to % body weight, and compare with the median of my
% device's Percent column over the same window.
%
% Michael Phelan, 2026

root = '/Users/michaelphelan/Desktop/LAB_DATA';
BW   = 787.3;              % my body weight, 177 lb in newtons

cond = {'featherweight','flat foot press','30 percent','50 percent', ...
        'stand on both feet','85 percent','stand on test foot only'};
file = {'my_data_featherweight.csv','my_data_flat_foot_press.csv', ...
        'my_data_30_percent.csv','my_data_50_percent.csv', ...
        'my_data_stand_on_both_feet.csv','my_data_85_percent.csv', ...
        'my_data_stand_on_test_foot_only.csv'};
pfol = {'lab_featherweight','lab_flat_foot_press','lab_30_percent', ...
        'lab_50_percent','lab_stand_on_both_feet','lab_85_percent', ...
        'lab_stand_on_test_foot_only'};
pfil = {'Michael_featherweight','Michael_flat_foot_press','Michael_30_percent', ...
        'Michael_50_percent','Michael_stand_on_both_feet','Michael_85_percent', ...
        'Michael_stand_on_test_foot_only'};

fprintf('\ncondition                 tr    true%%   device%%   ratio\n');
for c = 1:numel(cond)
    D = readtable(fullfile(root,cond{c},file{c}));
    for t = 1:3
        pf = fullfile(root,cond{c},pfol{c},sprintf('%s_trial_%d.forces',pfil{c},t));
        fz = readmatrix(pf,'FileType','text','NumHeaderLines',5);
        fz = fz(:,4);                               % column 4 = FZ, newtons

        % trim 1 s off the start and 2 s off the end, average the hold
        tp = mean(fz(2001:end-4000))/BW*100;

        % device: median over the middle 60% of the hold
        p  = D.Percent(D.Trial==t);
        dp = median(p(round(numel(p)*0.2):round(numel(p)*0.8)));

        fprintf('%-24s %3d %8.2f %9.2f %7.3f\n', cond{c}, t, tp, dp, dp/tp);
    end
end
