%% fit_calibration_curves.m
% Set two: fit new power-law calibration curves from the 24 bare-sensor
% presses, and compare them against the curves already in my firmware.
%
% Model: force_lb = a * raw^b, which is a straight line in log-log space.
% R^2 is reported on the pound values, not in log space, because log-space
% R^2 flatters the fit.

heel_raw = [421 412 444 328 318 321 283 285 300 244 229 228]';
heel_lb  = [45.0 48.2 42.1 78.9 89.2 78.8 112.3 109.3 111.9 164.0 162.4 162.9]';
ball_raw = [649 620 614 531 541 512 339 358 303 371 289 378]';
ball_lb  = [47.3 34.8 38.2 79.3 71.3 73.3 115.6 113.3 112.8 159.8 157.2 167.0]';

a_heel_old = 2.82e7;  b_heel_old = -2.22;     % curves in the firmware
a_ball_old = 9.83e7;  b_ball_old = -2.34;

p = polyfit(log(heel_raw),log(heel_lb),1);
bh = p(1); ah = exp(p(2));
R2h = 1 - sum((heel_lb - ah*heel_raw.^bh).^2)/sum((heel_lb-mean(heel_lb)).^2);

p = polyfit(log(ball_raw),log(ball_lb),1);
bb = p(1); ab = exp(p(2));
R2b = 1 - sum((ball_lb - ab*ball_raw.^bb).^2)/sum((ball_lb-mean(ball_lb)).^2);

fprintf('\nHEEL   new: %.4g * raw^(%.4f)   R^2 = %.4f     old: %.4g * raw^(%.4f)\n', ...
        ah,bh,R2h,a_heel_old,b_heel_old);
fprintf('BALL   new: %.4g * raw^(%.4f)   R^2 = %.4f     old: %.4g * raw^(%.4f)\n', ...
        ab,bb,R2b,a_ball_old,b_ball_old);

fprintf('\nraw      heel old   heel new   ball old   ball new\n');
for r = [400 500 600 700 800 900]
    fprintf('%4d   %9.1f  %9.1f  %9.1f  %9.1f\n', r, ...
        a_heel_old*r^b_heel_old, ah*r^bh, a_ball_old*r^b_ball_old, ab*r^bb);
end

figure('Color','w','Position',[100 100 1000 420]); rr = linspace(180,1000,400);
subplot(1,2,1); hold on; grid on; box on;
plot(heel_raw,heel_lb,'ko','MarkerFaceColor','k','DisplayName','measured');
plot(rr,ah*rr.^bh,'b-','LineWidth',1.6,'DisplayName',sprintf('new: %.3g \\cdot raw^{%.3f}',ah,bh));
plot(rr,a_heel_old*rr.^b_heel_old,'r--','LineWidth',1.6,'DisplayName',sprintf('old: %.3g \\cdot raw^{%.3f}',a_heel_old,b_heel_old));
xlabel('raw reading'); ylabel('true load (lb)'); ylim([0 250]);
title(sprintf('HEEL     new fit R^2 = %.4f',R2h)); legend('Location','northeast','FontSize',9);

subplot(1,2,2); hold on; grid on; box on;
plot(ball_raw,ball_lb,'ko','MarkerFaceColor','k','DisplayName','measured');
plot(rr,ab*rr.^bb,'b-','LineWidth',1.6,'DisplayName',sprintf('new: %.3g \\cdot raw^{%.3f}',ab,bb));
plot(rr,a_ball_old*rr.^b_ball_old,'r--','LineWidth',1.6,'DisplayName',sprintf('old: %.3g \\cdot raw^{%.3f}',a_ball_old,b_ball_old));
xlabel('raw reading'); ylabel('true load (lb)'); ylim([0 250]);
title(sprintf('BALL     new fit R^2 = %.4f',R2b)); legend('Location','northeast','FontSize',9);
