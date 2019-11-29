# Usage: ns2.tcl <TCP_flavor> <case_no> 

if {$argc != 2} {
	puts stderr "ERROR! ns2 called with wrong number of arguements! ($argc)"
	exit 1
} else {
	set flavor [lindex $argv 0]
	set case_no [lindex $argv 1]
}

#Create a simulator object
set ns [new Simulator]

# delay 
if {$case_no == 1} {
	set src1_r1_delay 5ms
	set r2_rcv1_delay 5ms
	set src2_r1_delay 12.5ms
	set r2_rcv2_delay 12.5ms
} elseif {$case_no == 2} {
	set src1_r1_delay 5ms
	set r2_rcv1_delay 5ms
	set src2_r1_delay 20ms
	set r2_rcv2_delay 20ms
} elseif {$case_no == 3} {
	set src1_r1_delay 5ms
	set r2_rcv1_delay 5ms
	set src2_r1_delay 27.5ms 
	set r2_rcv2_delay 27.5ms
} else {
	puts stderr "ERROR! Wrong case_no ($case_no)"
}


$ns color 1 Red 
$ns color 2 Blue

#Open the nam trace file
set f1 [open out1.tr w]
set f2 [open out2.tr w]
set nf [open out.nam w]
$ns trace-all $nf
$ns namtrace-all $nf


set src1 [$ns node]
set src2 [$ns node]
set r1 [$ns node]
set r2 [$ns node]
set rcv1 [$ns node]
set rcv2 [$ns node]


$ns duplex-link $src1 $r1 10Mb $src1_r1_delay DropTail
$ns duplex-link $src2 $r1 10Mb $src2_r1_delay DropTail
$ns duplex-link $r1 $r2 1Mb 5ms DropTail
$ns duplex-link $r2 $rcv1 10Mb $r2_rcv1_delay DropTail
$ns duplex-link $r2 $rcv2 10Mb $r2_rcv2_delay DropTail

$ns duplex-link-op $src1 $r1 orient right-down
$ns duplex-link-op $src2 $r1 orient right-up
$ns duplex-link-op $r1 $r2 orient right
$ns duplex-link-op $rcv1 $r2 orient left-down
$ns duplex-link-op $rcv2 $r2 orient left-up

if {[string compare $flavor "Sack"] == 0} {
	set tcp1 [new Agent/TCP/Sack1]
	$ns attach-agent $src1 $tcp1
	$tcp1 set fid_ 1

	set tcp2 [new Agent/TCP/Sack1]
	$ns attach-agent $src2 $tcp2
	$tcp2 set fid_ 2

	set sink1 [new Agent/TCPSink/Sack1]
	$ns attach-agent $rcv1 $sink1
	
	set sink2 [new Agent/TCPSink/Sack1]
	$ns attach-agent $rcv2 $sink2


} elseif {[string compare $flavor "Vegas"] == 0} {
	set tcp1 [new Agent/TCP/Vegas]
	$ns attach-agent $src1 $tcp1
	$tcp1 set fid_ 1

	set tcp2 [new Agent/TCP/Vegas]
	$ns attach-agent $src2 $tcp2
	$tcp2 set fid_ 2

	set sink1 [new Agent/TCPSink]
	$ns attach-agent $rcv1 $sink1

	set sink2 [new Agent/TCPSink]
	$ns attach-agent $rcv2 $sink2


} else {
	puts "ERROR! TCP version not supported. ($flavor)"
	exit 1
}

$ns connect $tcp1 $sink1
$ns connect $tcp2 $sink2

# setup ftp over tcp
set ftp1 [new Application/FTP]
$ftp1 attach-agent $tcp1

set ftp2 [new Application/FTP]
$ftp2 attach-agent $tcp2

# writes the data to the output files
proc record {} {
	global sink1 sink2 f1 f2
	set ns [Simulator instance]
	set time 0.5

	set bw1 [$sink1 set bytes_]
	set bw2 [$sink2 set bytes_]
	set now [$ns now]

	# calculate bw in Mbits and write to the file
	puts $f1 "$now [expr $bw1/$time*8/1000000]"
	puts $f2 "$now [expr $bw2/$time*8/1000000]"

	# reset the bytes_ values on the traffic sinks
	$sink1 set bytes_ 0
	$sink2 set bytes_ 0

	$ns at [expr $now+$time] "record"
}

#Define a 'finish' procedure
proc finish {} {
	global ns f1 f2 nf
	$ns flush-trace
	#Close the trace file
	close $f1
	close $f2
	close $nf
	#Execute nam on the trace file
	exec nam out.nam &
	exec xgraph out1.tr out2.tr -geometry 800x400 &
	exit 0
}

$ns at 0.5 "$ftp1 start"
$ns at 1.5 "$ftp2 start"
$ns at 4.0 "$ftp2 stop"
$ns at 4.5 "$ftp1 stop"

#Call the finish procedure after 5 seconds simulation time
$ns at 5.0 "finish"
$ns at 0.5 "record"

#Run the simulation
$ns run
