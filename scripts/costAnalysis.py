import sys
vout={"InitialScan":1,
	  "Scan":1,
	  "BeginGatherSearch":1,
	  "GatherSearch":1,
	  "RelativeGather":1}
voutscatter={"InitialScan":1,
			 "Scan":1,
			 "RelativeGather":1}
costs= {"InitialScan":(16,0,4,{"add":1,
							 "cmp":3}),
		"Scan":(8,0,4,{"add":1,
					 "cmp":1}),
		"BeginGatherSearch":(20,0,4,{"cmp":4,
								   "bit":1,
								   "mul":1}),
		"GatherSearch":(4,20,4,{"add":3,
								"cmp":3,
								"mul":1}),
		"RelativeGather":(4,16,16,{"add":1}),
		"processSlice":(8,0,16,{"cmp":10,
								"add":7,
								"mul":1}),
		"processTriangles":(4,48,36,{"div":1,
									 "mul":1,
									 "add":12,
									 "cmp":1}),
		"processFirstTriangles":(16,12,12,{"div":1,
										   "add":3}),
		"processTrianglesNoCompactOneOut":(16,12,12,{"div":1,
													 "add":3}),
		"produceTriP":(48,0,16,{"cmp":3,
								"bit":2}),
		"computeNeighbors":(8,192,144,{"mul":12*12+12*6,
									   "add":12*6+12*3,
									   "cmp":12*3+12,
									   "bit":12}),
		"splitTriangles":(8,192,96,{"add":60,
									"mul":63,
									"cmp":15,
									"bit":15,
									"div":6,
									"cos":3}),
		"ReorgSplitTriangles":(240,0,192,{"add":102,
										  "mul":12,
										  "div":1,
										  "cmp":23,
										  "bit":9}),
		"writeFinalTriangles":(36,0,12,{"cmp":2,
										"div":1}),
		"CheckTriangleCollide":(16,72,16,{"add":81,
										  "mul":73,
										  "div":4,
										  "bit":13,
										  "cmp":18}),
		"Collide":(64,128,32,{"add":70,
							  "mul":81,
							  "bit":18,
							  "cmp":16}),
		"updateCurrentNode":(16,192,64,{"add":66,
										"mul":84,
										"cmp":3,
										"bit":1})}
		
invocations={}
time = {}
alias = {"NanToBoolRight":"InitialScan",
		 "NanToRight":"Scan",
		 "linearReorgSplitTriangles":"ReorgSplitTriangles",
		 "EstablishGuess":"BeginGatherSearch",
		 "UpdateGuess":"GatherSearch"}

for arg in sys.argv[1:]:
	fp = open(arg,"r");
	lines = fp.readlines();
	for line in lines:
		line = line.strip();
		where=line.find(' ')
		if (where!=-1):
			nam = line[0:where]
			num = int(line[where+1:])
			if (nam in alias):
				nam = alias[nam];
			if not nam in invocations:
				invocations[nam]=0;
			invocations[nam]+=num;
	fp.close();
	voutreads=0
	voutwrites=0
	voutarith=0
	voutmem=0
	voutscatreads=0
	voutscatwrites=0
	voutscatarith=0
	voutscatmem=0
	procreads=0
	procwrites=0
	procmem=0
	procarith=0
	for func in invocations:
		numpixels = invocations[func];
		memarith=costs[func];
		reads = numpixels*(memarith[0]+memarith[1]);
		writes = numpixels*memarith[2];
		mem = reads+writes;
		st= func +": rd: "+str(reads)+" wr: "+str(writes);
		st += " mem: "+str(mem);
		arith = 0
		for i in memarith[3]:
			arith+=memarith[3][i];
		arith = numpixels*arith;
		st += " arith: "+str(arith);
		if (func in vout):
			voutreads+=reads
			voutwrites+=writes
			voutmem+=mem
			voutarith+=arith
		if (func in voutscatter):
			voutscatreads+=reads
			voutscatwrites+=writes
			voutscatmem+=mem
			voutscatarith+=arith
		if (not func in vout):
			procreads+=reads
			procwrites+=writes
			procmem+=mem
			procarith+=arith
		#print st
	print arg+" "+str (procmem)+" "+str (voutmem)+" "+str (voutscatmem)+" "+str (procarith)+" "+str (voutarith)+" "+str (voutscatarith);
#	print "Totals"
#	print "Mem: "+str (procmem+voutmem)
#	print "Ath: "+str (procarith+voutarith)
#	print ""
#	print "Proc"
#	print "Mem Arith: "+str (procmem)+" "+str (procarith)
#	print ""
#	print "Vout"
#	print "Mem Arith: "+str (voutmem)+" "+str (voutarith)
#	print ""
#	print "Vout Scatter"
#	print "Mem Arith: "+str (voutscatmem)+" "+str (voutscatarith)



