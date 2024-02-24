// void testBP()
// {
// 	uint32_t pc = 0x108;
// 	uint32_t target = 0x108;
// 	unsigned btbSize = 4;
// 	unsigned tagSize = 16;
// 	unsigned historySize = 4;
// 	uint32_t* ptr = new uint32_t(0x108);

// 	BranchPredictor bp(btbSize, historySize, tagSize, STRONGLY_TAKEN, false, false, 0);
// 	bp.print();
// 	bp.predict(pc, ptr);
// 	bp.update(pc, target, true, target);

// 	for(int i = 0; i < 32; i++)
// 	{
// 		if(i % 4 == 0)
// 			{bp.predict(pc, &target); bp.update(pc, target, true, target);}
// 		else
// 			{bp.predict(pc, &target); bp.update(pc, target, false, target);}
// 	}
// 	bp.print();
// 	delete ptr;
// }


// void testTableEntryLocalFsm()
// {
// 	uint32_t pc = 0x108;
// 	uint32_t target = 0x300;
// 	unsigned btbSize = 4;
// 	unsigned tagSize = 16;
// 	unsigned historySize = 4;
// 	TableEntry te = TableEntry(pc, target, btbSize, tagSize, historySize, STRONGLY_TAKEN);
// 	Fsm fsm = Fsm(STRONGLY_NOT_TAKEN);
// 	vector<Fsm> fsms(std::pow(2, historySize), fsm);
// 	TableEntry te1 = TableEntry(pc, target, btbSize, tagSize, historySize, fsms);
// 	if(te.predict())
// 		cout << "te predicts taken";
// 	else
// 		cout << "te predicts not taken";
// 	if(te1.predict())
// 		cout << "te1 predicts taken";
// 	else
// 		cout << "te1 predicts not taken";
// }

// void testTableEntry()
// {
// 	uint32_t pc = 0x108;
// 	uint32_t target = 0x300;
// 	unsigned btbSize = 4;
// 	unsigned tagSize = 16;
// 	unsigned historySize = 4;
// 	unsigned* gHist;
// 	*gHist = 2;


// 	TableEntry gHistTe(pc, target, btbSize, tagSize, gHist, historySize, STRONGLY_NOT_TAKEN);
// 	TableEntry gHistTe2(pc, target, btbSize, tagSize, gHist, historySize, STRONGLY_NOT_TAKEN);
// 	TableEntry lHistTe(pc, target, btbSize, tagSize, nullptr, historySize, WEAKLY_NOT_TAKEN);
// 	for(int i = 0; i < 8; i++)
// 	{
// 		gHistTe.update(false);
// 		gHistTe2.update(true);
// 		lHistTe.update(true);
// 	}
// 	lHistTe.print();
// 	cout << lHistTe.getHistory() << endl;
// }

// void testFsms()
// {
// 	Fsm fsm(STRONGLY_TAKEN);
// 	cout << fsm.state << endl;
// 	fsm.update(false);
// 	cout << fsm.state << endl;
// 	cout << "prediction: " << (fsm.predict() ? "taken" : "not taken") << endl;
// 	fsm.update(false);
// 	cout << "prediction: " << (fsm.predict() ? "taken" : "not taken") << endl;
// }