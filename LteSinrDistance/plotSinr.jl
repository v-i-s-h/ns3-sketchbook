# Plot Sinr v/s Distance(Time) for UE traces

using PyPlot

traceFilename = "./ue1Traces_60KMPH.txt"

# Read
traceData   = readdlm( traceFilename, '\t' )

figure();
plot( 16.67*traceData[:,1], 10*log(traceData[:,3]) );
xlabel( "Distance(m)" );
ylabel( "SINR(dB)" );
title( "SINR v/s Distance($traceFilename)" );
