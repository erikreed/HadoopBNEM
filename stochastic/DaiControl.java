class DaiControl {


	private native void readInFactorgraph(String path);
	private native void readInEvidence(String path);
	private native void readInEMfile(String path);
	private native void prepEM();
	private native double runEM(int numIterations);

	static {
		System.load("/data01/Projects/David_and_Erik/bullshit/ml/libdai/stochastic/DaiControl.so");
	}

	public static void main(String[] args) {
		DaiControl dai = new DaiControl();
		dai.readInFactorgraph("asd");
		dai.readInEvidence("asd");
		dai.readInEMfile("asd");

		dai.prepEM();
		dai.runEM(5);
	}

}
