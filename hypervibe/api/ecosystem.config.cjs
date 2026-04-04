//this is a pm2 config File.apply
//to start it, run npx pm2 start ecosystem.config.cjs

// installation of pm2: npm install pm2 -g
// pm2 startup # to generate the command to run on startup
// pm2 save # to save the current process list so it can be resurrected on startup

const config = {
  apps : [
        {
            name   : "apiserver",
            script : "/Users/yoren/Documents/gitprojecten/Hyperion2_projects/hypervibe/api/server.js",
            watch: true
        },
    ]
}   

module.exports = config;
// export default config;