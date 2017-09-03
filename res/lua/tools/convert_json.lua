local convertCommand = _toolsRoot .. "/build/external/json-to-gameplay3d/json2gp3d"

function convert(resourceDirName, extension)
  local rawDir = _toolsRoot .. "/raw/" .. resourceDirName
  local resDir = _toolsRoot .. "/res/" .. resourceDirName
  for index,fileName in pairs(ls(rawDir)) do
      if string.match(fileName, "%.json") and not string.match(fileName, "rules.json") then
        local rawPath = rawDir .. "/" .. fileName
        local resPath = resDir .. "/" .. string.gsub(fileName, ".json", extension)
        local command = convertCommand .. " -i " .. rawPath .. " -o " .. resPath .. " -r " .. rawDir .. "/rules.json"
        print(command)
        os.execute(command)
      end
  end
end

convert("spritesheets", ".ss")
convert("levels", ".level")
