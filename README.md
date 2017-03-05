Sampleapp is an example of how to use protocol in program.

Getting working sampleapp you need to follow these steps:
    1) make all dependecies
      a) go to the ./third-party/RF24
      b) run "make all" then "sudo make install"
      c) do the same things for ./third-party/RF24Network
      d) do the same things for ./third-party/RF24Mesh
    2) make sampleapp
      a) go to the root of project
      b) run make all

You can also copy-paste lines below:
cd ./third-party/RF24
make all
sudo make install
cd ../RF24Network
make all
sudo make install
cd ../RF24Mesh
make all
sudo make install
cd ../../
make all

Run sampleapp:
sudo ./bin/sampleapp <unique id>
Example:
<pre><code>
sudo ./bin/sampleapp 3
</code></pre>
