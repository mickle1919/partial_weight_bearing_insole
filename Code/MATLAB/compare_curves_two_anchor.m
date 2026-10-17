%% compare_curves_two_anchor.m
% Would the new calibration curves have scored better than the old ones?
%
% The device stores baseline and fullWeight already converted into pounds
% and discards the raw samples, so they cannot be recomputed under a
% different curve. Instead this solves for both constants using the two
% conditions whose true answers are known independently (featherweight and
% standing on the test foot only), then scores the other 15 trials under
% each curve set. Those 15 were not used in the solve.
%
% Michael Phelan, 2026

root = '/Users/michaelphelan/Desktop/LAB_DATA';

cond = {'featherweight','flat foot press','30 percent','50 percent', ...
        'stand on both feet','85 percent','stand on test foot only'};
file = {'my_data_featherweight.csv','my_data_flat_foot_press.csv', ...
        'my_data_30_percent.csv','my_data_50_percent.csv', ...
        'my_data_stand_on_both_feet.csv','my_data_85_percent.csv', ...
        'my_data_stand_on_test_foot_only.csv'};

truth = [ 8.91 10.87 14.90; 22.68 23.18 22.29; 29.82 27.64 27.79; ...
         50.22 49.49 49.60; 49.53 49.55 49.57; 76.58 79.11 77.88; ...
         98.33 97.29 97.64];

a_h_old=2.82e7;  b_h_old=-2.22;    a_b_old=9.83e7;   b_b_old=-2.34;
a_h_new=2.08e7;  b_h_new=-2.1518;  a_b_new=2.031e6;  b_b_new=-1.6539;

rawH = nan(7,3); rawB = nan(7,3);
for c = 1:7
    T = readtable(fullfile(root,cond{c},file{c}));
    for t = 1:3
        m = T.Trial==t; h = T.Heel_avg(m); b = T.Ball_avg(m);
        i1 = round(numel(h)*0.2); i2 = round(numel(h)*0.8);
        rawH(c,t) = median(h(i1:i2));  rawB(c,t) = median(b(i1:i2));
    end
end

S_old = a_h_old*rawH.^b_h_old + a_b_old*rawB.^b_b_old;
S_new = a_h_new*rawH.^b_h_new + a_b_new*rawB.^b_b_new;

bands = @(x) 1*(x<10) + 2*(x>=10&x<30) + 3*(x>=30&x<50) + 4*(x>=50&x<85) + 5*(x>=85);
lbl = {'OLD curves','NEW curves'};

for k = 1:2
    if k==1, S = S_old; else, S = S_new; end
    Sf = mean(S(1,:)); Tf = mean(truth(1,:))/100;    % featherweight anchor
    St = mean(S(7,:)); Tt = mean(truth(7,:))/100;    % test-foot anchor
    span = (St - Sf)/(Tt - Tf);                       % = fullWeight - baseline
    B    = Sf - Tf*span;
    F    = B + span;
    pct  = (S - B)./span * 100;

    idx = [2 3 4 5 6];                                % the 15 untouched trials
    tv = truth(idx,:); pv = pct(idx,:);
    tb = bands(tv(:)); pb = bands(pv(:));

    fprintf('\n=== %s ===\n  baseline %.2f lb   fullWeight %.2f lb\n',lbl{k},B,F);
    fprintf('  bands %d/15   under-banded %d   MAE %.2f pts\n', ...
            sum(tb==pb), sum(pb<tb), mean(abs(pv(:)-tv(:))));
    for c = 1:numel(idx)
        fprintf('   %-24s true %6.2f %6.2f %6.2f  |  dev %7.2f %7.2f %7.2f\n', ...
            cond{idx(c)}, tv(c,1),tv(c,2),tv(c,3), pv(c,1),pv(c,2),pv(c,3));
    end
end
