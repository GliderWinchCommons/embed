/* File: README.txt -- for the tunnel_scripts directory
 */

There are 3 tunnel scripts: tunnel-startup and tunnel-shutdown do the
obvious functions.  Tunnel-running determines if the tunnel is up or
not and prints a message on stdout.  Terminal-running can be used with
the "-z" switch to get it to print out just the pid.  This is useful
within other scripts.

If you have a Linux box that has access to the FIT PC (via ssh), then
you can start the tunnel from the FIT PC to your Linux box.  An
administrator logged into the FIT PC can log into your Linux box with
the following command:

    $ ssh -p 22222 gsm@localhost
    
Note that tunnel-startup assumes you have an account on the FIT PC with
the same account name as you're logged into on your Linux box.

--

09/10/2012
More notes:

Three machines involved:
remote machine ubuntu
FIT server 
my machine deh

Remote machine starts tunnel--
tunnel-startup

This connects to the FIT server, ubuntu account.  For this to work the
remote machine must have the a rsa public key added "authorized_keys" in
FIT PC, /home/ubuntu/.ssh directory.

My machine ssh's into the FIT PC ubuntu account--
ssh -p 41574 -l ubuntu $FITIP

For this to work, "my machine" must have the private key in /home/deh/.ssh
and the FIT PC must have the public key of the pair in /home/ubuntu/,ssh
added to "authorized_keys".

My machine logged into the FIT PC as ubuntu, then connects to the tunnel
ssh -p 22222 ubuntu@localhost 

If successful it asks for the password of the remote machine, e.g.
DATA LOGGER

If the "known_hosts" changed, then the offending "line" from the "known_hosts"
file must be deleted.  The next attempt will then ask for a yes/no to allow and
then, given a yes, adds the host. and the remote host password will be requested.

09/12/2012

Executing svn (or ssh) on remote machine--

For svn to work the remote machine has to have ~/.subversion/config
file changed so that the rsa key specified (-i switch).  It doesn't pick up the default when 
ssh is initiated via the reverse ssh tunnel.  

Example--
sshtunnelFIT = ssh -q -p 41574 -l deh -i /home/deh/AOA150-1

sshtunnelFIT = ssh -q -p 41574 -l deh [Does NOT work]
sshtunnelFIT = ssh -q -p 41574 -l deh -i ~/AOA150-1 [Does NOT work]

To ssh out of the remote machine back into the FIT--
ssh -p 41574 -l deh $FITIP -i /home/deh/AOA150-1

=============================================================================
10/13/2018 Update notes

Remote machine behind NAT, e.g. George's machine where his router/access-point cannot
be setup with a port forward (without wrestling with his ISP), or Uli using a
netbook.

The remote machine executes a reverse ssh connection to the FIT server, e.g.

ssh -R 22222:localhost:22 georgesm@47.204.208.132 -p 41574
Or,
ssh -R 22222:localhost:22 georgesm@$FITIP -p 41574

Where:
22222 = Port on the FIT that will connect to the George's machine
22 = ssh listening port on George's machine, which is the defaut
     specified in the ssh configuration file--
  /etc/ssh/sshd_config
georgesm = account name on the FIT.  
47.204.208.132 is the IP for the FIT (or use $FITIP)
-p = ssh listening port number on the FIT

When the above is executed the remote machine should respond
indicating successful connection to the FIT-

At this point the localhost:22222 on the FIT connects to George's 
machine.

The local machine, e.g. my desktop, or Charlie's machine, connects 
to the FIT server, e.g.

ssh $FITIP -l $USER -p 41574 -A

Where
$FITIP holds the current IP address
=p is the port forwarding address for the FIT
-A allows a machine ssh'ing into the FIT to ssh out of the FIT

Once connected to the FIT, connect to the remote machine, e.g.

ssh -p 22222 gsm@localhost

A successful response looks something like the following and
asks for the password of the remote machine--

deh@FIT-PC2:~$ ssh -p 22222 gsm@localhost
gsm@localhost's password: 
Welcome to Ubuntu 16.04.5 LTS (GNU/Linux 4.15.0-36-generic x86_64)
[snip]
Last login: Sat Oct 13 17:14:14 2018 from 127.0.0.1

At this point the local machine (e.g. my desktop) terminal window used
to ssh into the FIT operates as a terminal on George's machine.

Additonal terminal windows can be opened by sshing into the FIT and
executing "ssh -p 22222 gsm@localhost".  Note this can be done by
others that have access to the FIT, e.g. Charlie and I can both have
essentially terminals on George's machine at the same time.

NOTE: George's initiation of the reverse ssh connection was to the
account on the FIT of 'georgesm' whereas his machine is 'gsm', hence
"ssh -p 22222 gsm@localhost" uses 'gsm'.

Multiple reverse sshing can be done, e.g. George's machine as outlined
above has the connection and the local machine (my desktop) is connected,
maybe even multiple times.  AND, the netbook can be setup.

The netbook sets up the reverse ssh with--

ssh -R 22223:localhost:41573 deh@$FITIP -p 41574

Note the difference in ports from the foregoing setup for George's
machine--
22223 = a port number different from George's reverse ssh
41573 = the listening port set on the "logger" netbook

When the local machine connects to the FIT and connects to the 
netbook it uses--

ssh -p 22223 deh@localhost

The netbooks and Toshiba can be used in place of the FIT server by
using the listening port (below) instead of the FIT 41574 (used
in the above examples).

==== 10/13/2018 =====
SSH PORTS
                       FIT    remote
              listen   local  name
FIT            41574   -----
Netbook-home   41576   22226  deh
Toshiba laptop 41575   22225  deh
Netbook-logger 41573   22223  deh
George's laptop   22   22222  gsm
ASUS laptop       22   22221  deh
George's desktop  22   22220  gsm(?)

Troubleshooting tips

1.  ssh into FIT OK, but reverse ssh fails

sshd (daemon) needs to be installed & running

Check
sudo service ssh status
or
cd /etc/ssh
ls [see if sshd_config is present]

To install--
sudo apt-get install openssh-server

2.  Too many authentication tries

cd /etc/ssh
sudo gedit sshd_config
Add line--
MaxAuthTries  30

3.  password not accepted when connecting to localhost port for
for remote machine.  <name> may be wrong--

ssh -p 22222 <name>@localhost

===== 10/18/2018 =======

Connecting to, e.g. netbook from desktop (ASUS laptop)

1.  On netbook setup reverse port to FIT

ssh -R 22226:localhost:41576 deh@$FITIP -p 41574

2.  On desktip (ASUS laptop) setup forward port to FIT

ssh -L 22223:localhost:22223 deh@$FITIP -p 41574

3.  On desktop get terminal on netbook via FIT

deh@ASUS:~$ ssh -p 22223 deh@localhost
[response] deh@AOA150-2:~$

4.  Transfiles with 'rsync'--

rsync -ruav -e 'ssh -p 22223' /home/deh/logcsa deh@localhost:/home/deh



