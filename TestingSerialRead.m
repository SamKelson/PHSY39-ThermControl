clear;


% create instance of arduino object
a = serialport('COM4', 9600);
fopen(a);

%set-up graph
h = animatedline;
% set axis using get current axis(gca)
ax = gca;
% set x-axis label
xlabel('Time(sec)');
% set x-axis label
ylabel('Voltage(V)');
% set title for graph
title('Potentiometer Voltage Graph');
% set y-axis limit from 0 to 5.2V
ax.YLim = [0 5.2];
% turn on grid
grid on;

% set start time
startTime = datetime('now');

while 1
    % Read current voltage value
    inSerial = jsondecode(fscanf(a));
    v = inSerial.avgVolt;
    % Get current time
    t =  inSerial.time * (1/1000);
    % Add points to animation
    addpoints(h,t,v);
    % Update X-axis limit
    ax.XLim = [t-10 t];
    drawnow limitrate;
end

%TODO: average voltage and use datetime from adurino 

fclose(a);
clear;
