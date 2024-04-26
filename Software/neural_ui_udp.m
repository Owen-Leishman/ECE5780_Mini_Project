clear all;
close all;
clc

% initialize a udp port (the IP address needs to be the computers)
u = udpport("IPV4","LocalHost","192.168.8.198","LocalPort",8000);
flush(u);
i = 0;


% allow the udpport to broadcast to every device on the network
u.EnableBroadcast = true;

% transmit a message to the device to set the stimulation to none
% (broadcasts to every device on the network)
write(u, [83, 0, 255],"uint8", "255.255.255.255" , 8000); 

% store the previous values for the sliders
prevWave = 0;
prevAmp  = 0;

% set the width of the window
windowWidth = 500;

% create a uifigure
uifig = uifigure;

% set the layout of the window
g = uigridlayout(uifig);
g.RowHeight = {'fit','0.2x','0.25x','0.2x','0.25x','fit','1x','fit','1x','fit','1x'};
g.ColumnWidth = {'1x','2x'};

% creat a dropdown to select the stimulation waveform 
dd = uidropdown(g,"Items",["None", "Square", "Sine"],"ItemsData",[0,1,2]);
dd.Layout.Row = 3;
dd.Layout.Column = 1;

% creaete a slider to select the stimulation amplitude
sld = uislider(g,"Limits",[0, 1], "Value", 0);
sld.Layout.Row = 5;
sld.Layout.Column = 1;


% create labels
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


% create all of the graphs and thier titles
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


% create the temperature label
temptitle = uilabel(g, "Text", "Temperature: ");
temptitle.FontSize = 24;
temptitle.HorizontalAlignment = "left";
temptitle.Layout.Row = 8;
temptitle.Layout.Column = 1;

% remove the x axis tick marks so that the figures don't change size when 
% they are updated
set(ax1,'XTick',[])
set(ax2,'XTick',[])
set(ax3,'XTick',[])
set(ax4,'XTick',[])
set(ax5,'XTick',[])


while true
    
    % read the first value of the message
    startIndex = char(read(u,1,"Char"));

    % if the message is properly alligned read the message
    if startIndex == 'S'

        % read temperature
        raw_temp = read(u,1,"uint8") * (2^8) + read(u,1,"uint8");
        
        % update the temperature label
        temptitle.Text = "Temperature: " + num2str(raw_temp * 0.0078125, '%.2f'); % will break if temp is negative
        
        % read the rest of the packet
        raw_packet = read(u,200,"uint16");
        
        if length(raw_packet) == 200

            % seperate the packet by channel
            channel_1 = raw_packet(1:40);
            channel_2 = raw_packet(41:80);
            channel_3 = raw_packet(81:120);
            channel_4 = raw_packet(121:160);
            channel_5 = raw_packet(161:200);

            i = i + 1;
            
            % convert all of the packet values to adc values
            for x = 1:length(channel_1)
                adc_val_1(((i - 1)*40) + x) = channel_1(1, x)/15000 - 1;
                adc_val_2(((i - 1)*40) + x) = channel_2(1, x)/15000 - 1;
                adc_val_3(((i - 1)*40) + x) = channel_3(1, x)/15000 - 1;
                adc_val_4(((i - 1)*40) + x) = channel_4(1, x)/15000 - 1;
                adc_val_5(((i - 1)*40) + x) = (channel_5(1, x) - 16000) / 4000;
            end


            %update graphs
            plot(ax1, 1:length(adc_val_1),adc_val_1);
            ax1.set('xlim',[length(adc_val_1) - windowWidth,length(adc_val_1)]);

            plot(ax2, 1:length(adc_val_2),adc_val_2);
            ax2.set('xlim',[length(adc_val_2) - windowWidth,length(adc_val_2)]);

            plot(ax3, 1:length(adc_val_3),adc_val_3);
            ax3.set('xlim',[length(adc_val_3) - windowWidth,length(adc_val_3)]);
           
            plot(ax4, 1:length(adc_val_4),adc_val_4);
            ax4.set('xlim',[length(adc_val_4) - windowWidth,length(adc_val_4)]);

            plot(ax5, 1:length(adc_val_5),adc_val_5);
            ax5.set('xlim',[length(adc_val_5) - (windowWidth/4),length(adc_val_5)]);
            

            % if the slider or dropdown have changed transmit the message
            if((dd.Value ~= prevWave) || (sld.Value ~= prevAmp))
                amp = uint8(sld.Value * 255);
                write(u, [83, dd.Value, amp],"uint8", "255.255.255.255" , 8000);
                prevWave = dd.Value;
                prevAmp = sld.Value;
            end
            
            
        else
            % if the packet is unaligned flush the queue
            u.flush();
        end
    end
end