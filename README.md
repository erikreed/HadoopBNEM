_erikreed@cmu.edu_

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

1. Install Hadoop 1.0.0

2. Set up environment variables (e.g.)
   export PLATFORM=/home/erik/hadoop/hadoop-1.0.0
   export HADOOP_INSTALL=/home/erik/hadoop/hadoop-1.0.0

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
