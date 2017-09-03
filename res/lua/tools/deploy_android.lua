os.execute(Game.getInstance():getConfig():getString("ant_dir") .. "/" .. "ant -buildfile ../android/build.xml debug install")
