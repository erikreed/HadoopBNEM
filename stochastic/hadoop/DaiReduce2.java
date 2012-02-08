
import java.io.File;
import java.io.IOException;
import java.net.URI;
import java.net.URISyntaxException;
import java.util.Iterator;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.conf.Configured;
import org.apache.hadoop.filecache.DistributedCache;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.DoubleWritable;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.SequenceFile;
import org.apache.hadoop.io.SequenceFile.CompressionType;
import org.apache.hadoop.mapred.FileInputFormat;
import org.apache.hadoop.mapred.FileOutputFormat;
import org.apache.hadoop.mapred.JobClient;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.MapReduceBase;
import org.apache.hadoop.mapred.Mapper;
import org.apache.hadoop.mapred.OutputCollector;
import org.apache.hadoop.mapred.Reducer;
import org.apache.hadoop.mapred.Reporter;
import org.apache.hadoop.mapred.SequenceFileInputFormat;
import org.apache.hadoop.util.Tool;
import org.apache.hadoop.util.ToolRunner;

public class DaiReduce2 extends Configured implements Tool {
    /** tmp directory for input/output */
    static private final Path TMP_DIR = new Path(
            DaiReduce2.class.getSimpleName() + "dai_temp");

    public static class DaiMapper extends MapReduceBase
        implements Mapper<LongWritable, IntWritable, LongWritable, DoubleWritable> {

            private native long createDai(String fg, String ev, String em);
            private native void prepEM(long data);
            private native double runEM(long data, int numIterations);
            private native void freeMem(long data);
            private native long copyDai(long data);
            private native void randomizeFG(long data);

            static {
                System.loadLibrary("daicontrol");
                //System.load(new File("libdaicontrol.so").getAbsolutePath());
            }

            private JobConf conf;
            private static Path fg_path = null;
            private static Path em_path = null;
            private static Path tab_path = null;
            private static Path lib_path = null;
            @Override
                public void configure(JobConf job) {
                    conf = job;
                    Path[] cacheFiles;
                    try {
                        cacheFiles = DistributedCache.getLocalCacheFiles(conf);
                        if (null != cacheFiles && cacheFiles.length > 0) {
                            for (Path cachePath : cacheFiles) {
                               //System.out.println(cachePath.getName());
                                if (cachePath.getName().equals("fg")) 
                                    fg_path = cachePath;
                                else if (cachePath.getName().equals("em")) 
                                    em_path = cachePath;
                                else if (cachePath.getName().equals("tab")) 
                                    tab_path = cachePath;
                            }
                        }
                    } catch (IOException e) {
                        System.err.println("IOException reading from distributed cache");
                        System.err.println(e.toString());
                    }

                }

            public void map(LongWritable seed,
                    IntWritable numIter,
                    OutputCollector<LongWritable, DoubleWritable> out,
                    Reporter reporter) throws IOException {


                DaiMapper dai = new DaiMapper();
                FileSystem fs = FileSystem.get(conf);
                //String s = fs.getWorkingDirectory().getName();
                long dai_data = dai.createDai(fg_path.toString(), 
                        tab_path.toString(), em_path.toString());
                dai.randomizeFG(dai_data);
                dai.prepEM(dai_data);
                double l = dai.runEM(dai_data, numIter.get());
                dai.freeMem(dai_data);

                out.collect(seed, new DoubleWritable(l));
            }
        }

    public static class DaiReducer extends MapReduceBase
        implements Reducer<LongWritable, DoubleWritable, LongWritable, DoubleWritable> {

            private JobConf conf;

            @Override
                public void configure(JobConf job) {
                    conf = job;
                }

            public void reduce(LongWritable seed,
                    Iterator<DoubleWritable> likelihood,
                    OutputCollector<LongWritable, DoubleWritable> output,
                    Reporter reporter) throws IOException {
                while (likelihood.hasNext())
                    output.collect(seed, likelihood.next());
            }

            /**
             * Reduce task done, write output to a file.
             */
            //		@Override
            //		public void close() throws IOException {
            //			//write output to a file
            //			Path outDir = new Path(TMP_DIR, "out");
            //			Path outFile = new Path(outDir, "reduce-out");
            //			FileSystem fileSys = FileSystem.get(conf);
            //			SequenceFile.Writer writer = SequenceFile.createWriter(fileSys, conf,
            //					outFile, LongWritable.class, LongWritable.class, 
            //					CompressionType.NONE);
            //			writer.append(new LongWritable(numInside), new LongWritable(numOutside));
            //			writer.close();
            //		}
        }

    public static void start_mapreduce(int numMaps, int numIter, JobConf jobConf
            ,Configuration conf) throws IOException {
        //setup job conf
        jobConf.setJobName(DaiReduce2.class.getSimpleName());

        jobConf.setInputFormat(SequenceFileInputFormat.class);

        jobConf.setOutputKeyClass(LongWritable.class);
        jobConf.setOutputValueClass(DoubleWritable.class);
        //jobConf.setOutputFormat(SequenceFileOutputFormat.class);
        jobConf.setMapperClass(DaiMapper.class);
        jobConf.setNumMapTasks(numMaps);

        jobConf.setReducerClass(DaiReducer.class);
        jobConf.setNumReduceTasks(1);

        // turn off speculative execution, because DFS doesn't handle
        // multiple writers to the same file.
        jobConf.setSpeculativeExecution(false);
        //http://developer.yahoo.com/hadoop/tutorial/module5.html might have better syntax for distr. cache

        //DistributedCache.addLocalFiles(jobConf, "libdaicontrol.so");
        //DistributedCache.addFileToClassPath(new Path(), jobConf);
        try{
            //			DistributedCache.addCacheFile(
            //					new URI("/data01/Projects/David_and_Erik/bullshit/ml/libdai/stochastic/libdaicontrol.so" +
            //							"#libdaicontrol.so"), jobConf); 
            //DistributedCache.addCacheFile(new URI("hdfs://host:port/libraries/libdaicontrol.so#libdaicontrol.so"),jobConf);


            DistributedCache.addCacheFile(new URI("hdfs://localhost:9000/user/erik/dat/em#em"),jobConf);
            DistributedCache.addCacheFile(new URI("hdfs://localhost:9000/user/erik/dat/fg#fgm"),jobConf);
            DistributedCache.addCacheFile(new URI("hdfs://localhost:9000/user/erik/dat/tab#tab"),jobConf);
            DistributedCache.addCacheFile(new URI("hdfs://localhost:9000/libraries/libdaicontrol.so#libdaicontrol.so"),jobConf);
        } catch (URISyntaxException e) {
            System.err.println(e);
        }
        DistributedCache.createSymlink(jobConf); 
        //setup input/output directories
        final Path inDir = new Path(TMP_DIR, "in");
        final Path outDir = new Path(TMP_DIR, "out");
        FileInputFormat.setInputPaths(jobConf, inDir);
        FileOutputFormat.setOutputPath(jobConf, outDir);

        final FileSystem fs = FileSystem.get(jobConf);
        //		if (fs.exists(TMP_DIR)) {
        //			throw new IOException("Tmp directory " + fs.makeQualified(TMP_DIR)
        //					+ " already exists.  Please remove it first.");
        //		}
        if (!fs.mkdirs(inDir)) {
            throw new IOException("Cannot create input directory " + inDir);
        }

        try {
            //generate an input file for each map task
            for(int i=0; i < numMaps; ++i) {
                final Path file = new Path(inDir, "part"+i);
                final LongWritable seed = new LongWritable(i);
                final IntWritable fnum_iter = new IntWritable(numIter);

                final SequenceFile.Writer writer = SequenceFile.createWriter(
                        fs, jobConf, file,
                        LongWritable.class, IntWritable.class, CompressionType.NONE);
                try {
                    writer.append(seed, fnum_iter);
                } finally {
                    writer.close();
                }
                System.out.println("Wrote input for Map #"+i);
            }

            //start a map/reduce job
            System.out.println("Starting Job");
            final long startTime = System.currentTimeMillis();
            JobClient.runJob(jobConf);
            final double duration = (System.currentTimeMillis() - startTime)/1000.0;
            System.out.println("Job Finished in " + duration + " seconds");
            //
            //			//read outputs
            //			Path inFile = new Path(outDir, "reduce-out");
            //			LongWritable numInside = new LongWritable();
            //			LongWritable numOutside = new LongWritable();
            //			SequenceFile.Reader reader = new SequenceFile.Reader(fs, inFile, jobConf);
            //			try {
            //				reader.next(numInside, numOutside);
            //			} finally {
            //				reader.close();
            //			}
        }
        finally {
            fs.delete(TMP_DIR, true);
        }
    }

    /**
     * Parse arguments and then runs a map/reduce job.
     * Print output in standard out.
     * 
     * @return a non-zero if there is an error.  Otherwise, return 0.  
     */
    public int run(String[] args) throws Exception {
        if (args.length != 2) {
            System.err.println("Usage: "+getClass().getName()+" <nMaps> <num_iter>");
            ToolRunner.printGenericCommandUsage(System.err);
            return -1;
        }

        final int nMaps = Integer.parseInt(args[0]);
        final int numIter = Integer.parseInt(args[1]);

        System.out.println("Number of Maps  = " + nMaps);
        System.out.println("Num_iter per Map = " + numIter);

        final JobConf jobConf = new JobConf(getConf(), getClass());
        //		System.out.println("Estimated value of Pi is "
        //				+ estimate(nMaps, nSamples, jobConf));
        start_mapreduce(nMaps, numIter, jobConf,getConf());
        return 0;
    }

    public static void main(String[] argv) throws Exception {
        System.exit(ToolRunner.run(null, new DaiReduce2(), argv));
    }
}
