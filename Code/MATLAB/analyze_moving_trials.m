%% analyze_moving_trials.m
% Set three: 4 moving conditions, 3 trials each (12 trials).
%
% The force plate captures one footfall; my device records ~5 s of several
% strides, and we started recording by voice so the two are not synced.
% This searches for the time shift that best lines the two signals up,
% then compares peaks and splits the stance phase into thirds.

root = '/Users/michaelphelan/Desktop/LAB_DATA';
BW   = 787.3;
cond = {'slow walk','walking normal','speed walk','run'};

figure('Color','w','Position',[60 60 1250 820]);
fprintf('\ncondition           tr  shift    r_st   plate%%  dev%%  ratio  early  mid   late  satN\n');
for ci = 1:numel(cond)
  cdir = fullfile(root,cond{ci});
  sub  = dir(cdir); sub = sub([sub.isdir] & ~startsWith({sub.name},'.'));
  fdir = fullfile(cdir,sub(1).name);
  cs   = dir(fullfile(cdir,'*.csv'));
  D    = readtable(fullfile(cdir,cs(1).name));

  for t = 1:3
    ff = dir(fullfile(fdir,sprintf('*trial_%d.forces',t)));
    fz = readmatrix(fullfile(fdir,ff(1).name),'FileType','text','NumHeaderLines',5);
    fz = fz(:,4);
    tp = (1:numel(fz))'/2000; yp = fz/BW*100;       % plate, 2000 Hz
    pd = D.Percent(D.Trial==t); td = (0:numel(pd)-1)'/53;   % device, ~53 Hz

    % put both on a common 200 Hz grid
    g  = (0:1/200:max([tp;td]))';
    Yp = interp1(tp,yp,g,'linear',0);
    Yd = interp1(td,pd,g,'linear',0);

    % search +/- 2 s for the shift that maximises correlation
    A = Yp - mean(Yp); nA = norm(A); best = -Inf; bl = 0;
    for L = round(-2*200):round(2*200)
        s = circshift(Yd,L); B = s - mean(s);
        r = (A'*B)/(nA*norm(B));
        if r > best, best = r; bl = L; end
    end
    Ysh = circshift(Yd,bl);

    % restrict to the plate's stance phase
    st = Yp > 5; i1 = find(st,1,'first'); i2 = find(st,1,'last');
    a = Yp(i1:i2); b = Ysh(i1:i2);
    r_st = ((a-mean(a))'*(b-mean(b)))/(norm(a-mean(a))*norm(b-mean(b)));
    n = numel(a); k = floor(n/3);
    e = mean(b(1:k))/mean(a(1:k));
    m = mean(b(k+1:2*k))/mean(a(k+1:2*k));
    l = mean(b(2*k+1:end))/mean(a(2*k+1:end));
    satN = sum(abs(fz-max(fz)) < 1);      % samples pinned at the plate's max

    fprintf('%-18s %3d %6.2f %7.3f %8.1f %6.1f %6.2f %6.2f %5.2f %5.2f %5d\n', ...
        cond{ci},t,bl/200,r_st,max(a),max(b),max(b)/max(a),e,m,l,satN);

    subplot(4,3,(ci-1)*3 + t); hold on; grid on; box on;
    plot(g,Yp,'b','LineWidth',1.1);
    plot(g,Ysh,'r','LineWidth',0.9);
    xlim([max(0,g(i1)-0.6) min(g(end),g(i2)+0.6)]); ylim([-20 220]);
    title(sprintf('%s T%d   r=%.2f  ratio %.2f',cond{ci},t,r_st,max(b)/max(a)),'FontSize',8);
    set(gca,'FontSize',7);
    if t==1, ylabel('%BW','FontSize',8); end
    if ci==numel(cond), xlabel('seconds','FontSize',8); end
  end
end
legend('force plate','my device','FontSize',7,'Location','northeast');
