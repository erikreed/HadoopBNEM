_erikreed@cmu.edu_

## Info
This code amends the libDAI library to provide:

* A new parameter learning implementation called **Age-Layered Expectation Maximization** (ALEM),
    inspired by the [ALPS](https://github.com/ghornby/alps) genetic algorithm work of Greg Hornby of NASA Ames.
* A distributed parameter learning implementation using MapReduce and a population structure,
    with either ALEM or a large random restart-equivalent which I call **Multiple Expectation Maximzation** (MEM).

This code can be used to recreate the results shown in my paper:

[Reed, E. and Mengshoel, O. “Scaling Bayesian Network Parameter Learning with Expectation Maximization using MapReduce”. In BigLearning Workshop of Neural Information Processing Systems (NIPS 2012).](http://fodava.gatech.edu/sites/default/files/FODAVA-12-19.pdf)

Additionally, this work can be used (with more emphasis on high performance computing) to recreate the algorithms suggested in the following two papers:

[Mengshoel, O. J., Saluja, A., & Sundararajan, P. (2012). Age-Layered Expectation Maximization for Parameter Learning in Bayesian Networks. In Proc. of the Fifteenth International Conference on Artificial Intelligence and Statistics (pp. 984-992).](http://works.bepress.com/cgi/viewcontent.cgi?article=1067&context=ole_mengshoel)

[A. Basak, I. Brinster, X. Ma, and O.J. Mengshoel. Accelerating Bayesian network parameter learning using Hadoop and MapReduce. In Proceedings of the 1st International Workshop on Big Data, Streams and Heterogeneous Source Mining: Algorithms, Systems, Programming Models and Applications, BigMine '12, pages 101–108, Beijing, China, 2012. ACM.](http://fodava.gatech.edu/sites/default/files/FODAVA-12-16.pdf)

## How to test using non-Hadoop local MapReduce

```bash
# working directory: ./mapreduce
make
./main.sh
```
---------------
## How to run using Hadoop locally

This is useful for testing purposes before deploying on
Amazon EC2 or another Hadoop cluster.

1. Install Hadoop (tested up to v1.1.1)

2. Set up environment variables (e.g.)
   export HADOOP_PREFIX=/home/erik/hadoop/hadoop-1.1.1

   Also set up Hadoop for pseudo-distributed operation: http://hadoop.apache.org/docs/r1.1.0/single_node_setup.html

3. Run the following (I have hadoop in my path):
  ```bash
  # working directory: .../libdai/mapreduce
  make clobber # WARNING: this clears any existing HDFS data
               # which seems to cause a bug sometimes when
               # accessing the HDFS
  hadoop namenode -format
  start-all
  ```

4. Initialize a BN and some data
  ```bash
  ./scripts/init dat/bnets/asia.net 100 4
  ```
5. Start Hadoop streaming
  ```bash
  make
  ./scripts/streaming dat/in
  ```

6. Gander at results
  ```bash
  ls out
  ```
7. Stop Hadoop
  ```bash
  stop-all
  ```
---------------
## How to run using Amazon EC2

1. ```$ make```

2. Launch an EC2 instance and send the mapreduce
   folder to it.
  ```bash
  scp -rCi ~/Downloads/dai-mapreduce.pem mapreduce ec2-user@ec2-107-21-71-50.compute-1.amazonaws.com:
  ```

3. ssh into the instance and launch a cluster.
  ```bash
  ssh -Ci ~/Downloads/dai-mapreduce.pem ec2-user@ec2-107-21-71-50.compute-1.amazonaws.com

  # Assumes hadoop-ec2 has been configured.
  # Launch a cluster of 10 (small) instances
  hadoop-ec2 launch-cluster dai-mapreduce 10
  # ... wait a really long time for the cluster to initialize
  ```

4. Push the mapreduce folder to the cluster master.
  ```bash
  hadoop-ec2 push dai-mapreduce mapreduce
  ```

5. Login to the cluster master.
  ```bash
  hadoop-ec2 login dai-mapreduce
  ```

6. Initialize a big bnet and start Hadoop streaming
  ```bash
  ./scripts/init dat/bnets/water.net 1000 10
  # Do standard EM, pop-size=10, mappers=10
  ./scripts/streaming dat/in -u 10 10
  # ... wait a few hours
  ```

7. Collect data!
  ```bash
  ls out
  ```
