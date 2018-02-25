# Plot data from DlRsrpSinrStats

using PyPlot


# Load Data
data, header = readdlm( "./DlRsrpSinrStats.txt", '\t', header=true );

noOfUsers   = unique( data[:,3] );
noOfEnbs    = unique( data[:,2] );

# Extract data for users -- assuming 2+2 users per Cell
data_u1 = data[ data[:,3].==1, : ];
data_u2 = data[ data[:,3].==2, : ];
data_u3 = data[ data[:,3].==3, : ];
data_u4 = data[ data[:,3].==4, : ];

# Plot data
figure();
plot( data_u1[:,1], 10*log(data_u1[:,6]), label = "User 1" );
plot( data_u2[:,1], 10*log(data_u2[:,6]), label = "User 2" );
plot( data_u3[:,1], 10*log(data_u3[:,6]), label = "User 3" );
plot( data_u4[:,1], 10*log(data_u4[:,6]), label = "User 4" );
grid();
ylabel( "SINR(dB)" );
xlabel( "Time" );
legend();
