class DaiControl {


	//private native void readInFactorgraph(String path);
	//private native void readInEvidence(String path);
	//private native void readInEMfile(String path);
	private native long  prepEM(String fg, String ev, String em);
	private native double runEM(long data, int numIterations);
	private native void freeMem(long data);

	static {
		System.load("/data01/Projects/David_and_Erik/bullshit/ml/libdai/stochastic/DaiControl.so");
	}

	public static void main(String[] args) {
		DaiControl dai = new DaiControl();

		long ptr = dai.prepEM("dat/fg", "dat/tab", "dat/em");
		
		double l= dai.runEM(ptr, 5);
		System.out.println("l = " + l);

		dai.freeMem(ptr);
	}

}
