# Plot data from DlRsrpSinrStats
using PyPlot

# Column Definitions in log file
const COL_TIME      = 1
const COL_USER_ID   = 3
const COL_RSRP      = 5
const COL_SINR      = 6

# Load Data
data, header = readdlm( "./logs/DlRsrpSinrStats.txt", '\t', header=true );

ueIds   = Integer.(unique( data[:,3] ));
enbIds  = Integer.(unique( data[:,2] ));

# Plot data
f_rsrp  = figure();
f_sinr  = figure();
# Plot SINR for all users
timeIdx = data[ data[:,COL_USER_ID].==ueIds[1], COL_TIME ];   # assuming time points will be same for all users
for ueId ∈ ueIds
    userRowIdx  = data[:,COL_USER_ID].==ueId
    sinr    = 10 * log.( data[userRowIdx,COL_SINR] );
    rsrp    = 10 * log.( 1000*data[userRowIdx,COL_RSRP] );
    figure( f_sinr[:number] );  plot( timeIdx, sinr, label = @sprintf("User %2d",ueId) );
    figure( f_rsrp[:number] );  plot( timeIdx, rsrp, label = @sprintf("User %2d",ueId) );
end
figure( f_sinr[:number] )
    grid(); legend();
    ylabel( "SINR(dB)" );   xlabel( "Time" );
figure( f_rsrp[:number] )
    grid(); legend();
    ylabel( "RSRP(dBm)" );   xlabel( "Time" );
