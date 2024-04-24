clear all;
close all;
clc

u = udpport("IPV4","LocalHost","192.168.8.198","LocalPort",8000);
flush(u);
i = 0;

u.EnableBroadcast = true;

% set stim to none by default
write(u, [83, 0, 255],"uint8", "255.255.255.255" , 8000); 

prevWave = 0;
prevAmp  = 0;

windowWidth = 2000;

uifig = uifigure;
g = uigridlayout(uifig);
g.RowHeight = {'fit','0.2x','0.25x','0.2x','0.25x','fit','1x','fit','1x','fit','1x'};
g.ColumnWidth = {'1x','2x'};

dd = uidropdown(g,"Items",["None", "Square", "Sine"],"ItemsData",[0,1,2]);
dd.Layout.Row = 3;
dd.Layout.Column = 1;

sld = uislider(g,"Limits",[0, 1], "Value", 0);
sld.Layout.Row = 5;
sld.Layout.Column = 1;

stimtitle = uilabel(g, "Text", "Stimulation");
stimtitle.FontSize = 24;
stimtitle.HorizontalAlignment = "left";
stimtitle.Layout.Row = 1;
stimtitle.Layout.Column = 1;

wavetitle = uilabel(g, "Text", "Waveform");
wavetitle.FontSize = 16;
wavetitle.HorizontalAlignment = "left";
wavetitle.Layout.Row = 2;
wavetitle.Layout.Column = 1;

amptitle = uilabel(g, "Text", "Amplitude (mA)");
amptitle.FontSize = 16;
amptitle.HorizontalAlignment = "left";
amptitle.Layout.Row = 4;
amptitle.Layout.Column = 1;

stitle = uilabel(g, "Text", "Current Measurement");
stitle.FontSize = 24;
stitle.HorizontalAlignment = "left";
stitle.Layout.Row = 6;
stitle.Layout.Column = 1;


ax5 = uiaxes(g);
ax5.Layout.Row = 7;
ax5.Layout.Column = 1;

ax1 = uiaxes(g);
ax1.Layout.Row = [2 5];
ax1.Layout.Column = [2 4];

ax1title = uilabel(g, "Text", "Channel 1");
ax1title.FontSize = 24;
ax1title.HorizontalAlignment = "center";
ax1title.Layout.Row = 1;
ax1title.Layout.Column = [2 4];

ax2 = uiaxes(g);
ax2.Layout.Row = 7;
ax2.Layout.Column = [2 4];

ax2title = uilabel(g, "Text", "Channel 2");
ax2title.FontSize = 24;
ax2title.HorizontalAlignment = "center";
ax2title.Layout.Row = 6;
ax2title.Layout.Column = [2 4];

ax3 = uiaxes(g);
ax3.Layout.Row = 9;
ax3.Layout.Column = [2 4];

ax3title = uilabel(g, "Text", "Channel 3");
ax3title.FontSize = 24;
ax3title.HorizontalAlignment = "center";
ax3title.Layout.Row = 8;
ax3title.Layout.Column = [2 4];

ax4 = uiaxes(g);
ax4.Layout.Row = 11;
ax4.Layout.Column = [2 4];

ax4title = uilabel(g, "Text", "Channel 4");
ax4title.FontSize = 24;
ax4title.HorizontalAlignment = "center";
ax4title.Layout.Row = 10;
ax4title.Layout.Column = [2 4];



temptitle = uilabel(g, "Text", "Temperature: ");
temptitle.FontSize = 24;
temptitle.HorizontalAlignment = "left";
temptitle.Layout.Row = 8;
temptitle.Layout.Column = 1;



while true
    startIndex = char(read(u,1,"Char"));
    if startIndex == 'S'
        raw_temp = read(u,1,"uint8") * (2^8) + read(u,1,"uint8");
        temptitle.Text = "Temperature: " + num2str(raw_temp * 0.0078125, '%.2f'); % will break if temp is negative
        
        
        raw_packet = read(u,100,"int16");
        
        if length(raw_packet) == 100
            i = i + 1;
            
            for x = 1:100
                
                adc_val(((i - 1)*100) + x) = (raw_packet(1, x) * 1.6 * 1000 * 10/11) / (32768 * 16);
            end

            %update graphs
            plot(ax1, 1:length(adc_val),adc_val);
            %set(lHandle, 'XData',1:length(adc_val), 'YData', adc_val);
            ax1.set('xlim',[length(adc_val) - windowWidth,length(adc_val)]);

            if((dd.Value ~= prevWave) || (sld.Value ~= prevAmp))
                amp = uint8(sld.Value * 255);
                write(u, [83, dd.Value, amp],"uint8", "255.255.255.255" , 8000);
                prevWave = dd.Value;
                prevAmp = sld.Value;
            end
            
            
        else
            u.flush();
        end
    end
end