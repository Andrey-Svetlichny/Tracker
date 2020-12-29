# create server on Amazon

## create directory
sudo -s
mkdir -p /var/node/tracker
chown -R ec2-user /var/node/tracker
chmod -R 755 /var/node/tracker
exit

### deploy server.js and package.json to /var/node/tracker

## start
cd /var/node/tracker
pm2 start ./server.js -n tracker
