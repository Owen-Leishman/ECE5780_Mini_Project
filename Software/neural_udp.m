clear all;
close all;
clc

u = udpport("IPV4","LocalHost","192.168.8.198","LocalPort",8000);
flush(u);
i = 0;

windowWidth = 2000;

figure(1);
%t = tiledlayout(1,2);
ax1 = axes;
title("Channel 1 Readings");
ylabel("Voltage (mV)");
lHandle = line(nan, nan);
xlabel("Time (samples)");

subtitle("Temperature: ");

while true
    startIndex = char(read(u,1,"Char"));
    if startIndex == 'S'
        raw_temp = read(u,1,"uint8") * (2^8) + read(u,1,"uint8");
        subtitle("Temperature: " + num2str(raw_temp * 0.0078125, '%.2f')); % will break if temp is negative
        raw_packet = read(u,100,"int16");
        %raw_packet = read(u,200,"uint8");
        if length(raw_packet) == 100
            i = i + 1;
            %packet(i,:) = raw_packet(1:200);
            for x = 1:100
                %adc_val(((i - 1)*100) + x) = uint32(raw_packet(1, (x - 1) * 2 + 1) * (2^8)) + raw_packet(1, (x) * 2);
                adc_val(((i - 1)*100) + x) = (raw_packet(1, x) * 1.6 * 1000 * 10/11) / (32768 * 16);
            end

            
            set(lHandle, 'XData',1:length(adc_val), 'YData', adc_val);

            ax1.set('xlim',[length(adc_val) - windowWidth,length(adc_val)]);
        else
            u.flush();
        end
    end
end