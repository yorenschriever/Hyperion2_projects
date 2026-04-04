//this is a pm2 config File.apply
//to start it, run npx pm2 start ecosystem.config.cjs

// installation of pm2: npm install pm2 -g
// pm2 startup # to generate the command to run on startup
// pm2 save # to save the current process list so it can be resurrected on startup

// This starts a server in port 3000
// nginx will combine it with static files in this config
// /usr/local/etc/nginx/nginx.conf

// a cloudflare tunnel will expose the nginx server to the internet, and forward requests to the express server

const config = {
  apps : [
        {
            name   : "apiserver",
            script : "./server.js",
            watch: true
        },
    ]
}   

module.exports = config;
// export default config;