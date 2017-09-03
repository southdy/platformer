function getVector2(properties, name, defaultX, defaultY)
    local vec2 = Vector2.new(defaultX, defaultY)
    if properties:exists(name) then
        properties:getVector2(name, vec2)
    end
    return vec2
end

local inkscapePixelToRealPixelScale = 0.2 -- no idea why
local commands = {}
local propertiesRoot = Properties.create(_toolsRoot .. "/raw/svgs/texture.export")
local spriteSheetProperties = propertiesRoot:getNextNamespace()
while spriteSheetProperties do
    local outputDir = _toolsRoot .. "/raw/textures/" .. spriteSheetProperties:getNamespace()
    mkdir(outputDir)
    local annonNameIndex = 0
    local frameGroupProperties = spriteSheetProperties:getNextNamespace()
    while frameGroupProperties do
        local frameProperties = frameGroupProperties:getNextNamespace()
        while frameProperties do
            local svgFilePath = "\"" .. _toolsRoot .. "/raw/svgs/" .. frameProperties:getString("svg") .. "\""

            local region = Vector4.new()
            frameProperties:getVector4("region", region)
            local size =  getVector2(frameProperties, "size", region:z(), region:w())
            local offset =  getVector2(frameProperties, "offset", size:x(), size:y())
            local scale = getVector2(frameProperties, "scale", 1, 1);
            local padding = getVector2(frameProperties, "padding", 1, 1)
            local marginId = frameProperties:getString("marginId")
            padding:set(padding:x() * inkscapePixelToRealPixelScale, padding:y() * inkscapePixelToRealPixelScale)

            local textureWidth = (size:x() * scale:x()) + (padding:x() * 2)
            local textureHeight = (size:y() * scale:y()) + (padding:y() * 2)
            local startX = region:x()
            local x = startX
            local y = region:y()

            while ((y - offset:y()) >= (region:y() - region:w())) do
                while ((x + offset:x()) <= (region:x() + region:z())) do
                    local command = {}
                    command.padding = padding
                    command.svgFilePath = svgFilePath
                    command.outputDir = outputDir
                    command.marginId = marginId
                    command.imgTopLeftX = x
                    command.imgTopLeftY = y
                    command.imgBottomRightX = x + size:x()
                    command.imgBottomRightY = y - size:y()
                    command.textureWidth = textureWidth
                    command.textureHeight = textureHeight
                    command.namespace = frameGroupProperties:getNamespace()
                    command.id = frameProperties:getId()
                    if command.namespace == "batch" then
                        command.namespace = ""
                        command.id = command.id .. tostring(annonNameIndex)
                        annonNameIndex = annonNameIndex + 1
                    end
                    table.insert(commands, command)
                    x = x + offset:x()
                end
                y = y - offset:y()
                x = startX
            end
            frameProperties = frameGroupProperties:getNextNamespace()
        end
        frameGroupProperties = spriteSheetProperties:getNextNamespace()
    end

	spriteSheetProperties = propertiesRoot:getNextNamespace()
end

local marginToLargestTextureDimensions = {}
for index, command in pairs(commands) do
    if command.marginId and string.len(command.marginId) > 0 then
        if not marginToLargestTextureDimensions[command.marginId] then
            marginToLargestTextureDimensions[command.marginId] = Vector2.new()
        end
        local largestTextureDimensions = marginToLargestTextureDimensions[command.marginId]
        if largestTextureDimensions:x() < command.textureWidth then
            largestTextureDimensions:set(command.textureWidth, largestTextureDimensions:y())
        end
        if largestTextureDimensions:y() < command.textureHeight then
            largestTextureDimensions:set(largestTextureDimensions:x(), command.textureHeight)
        end
    end
end

local inkscapeCmd = "\"" .. Game.getInstance():getConfig():getString("inkscape_dir") .. "/inkscape"
if Game.getInstance():getConfig():getString("os") == "windows" then
    inkscapeCmd = "CALL " .. inkscapeCmd .. ".com"
end
inkscapeCmd = inkscapeCmd  .. "\""

for index, command in pairs(commands) do
    local largestTextureDimensions = marginToLargestTextureDimensions[command.marginId]
    if largestTextureDimensions and (largestTextureDimensions:x() ~= command.textureWidth or largestTextureDimensions:y() ~= command.textureHeight) then
        local marginHorizonal = ((largestTextureDimensions:x() - command.textureWidth) / 2) * inkscapePixelToRealPixelScale
        local marginVertical = ((largestTextureDimensions:y() - command.textureHeight) / 2) * inkscapePixelToRealPixelScale
        command.imgTopLeftX = command.imgTopLeftX - marginHorizonal
        command.imgTopLeftY = command.imgTopLeftY + marginVertical
        command.imgBottomRightX = command.imgBottomRightX + marginHorizonal
        command.imgBottomRightY = command.imgBottomRightY
        command.textureWidth = largestTextureDimensions:x()
        command.textureHeight = largestTextureDimensions:y()
    end

    command.imgTopLeftX = command.imgTopLeftX - (command.padding:x() * 2)
    command.imgTopLeftY = command.imgTopLeftY + (command.padding:y() * 2)
    command.imgBottomRightX = command.imgBottomRightX + command.padding:x()
    command.imgBottomRightY = command.imgBottomRightY - command.padding:y()

    local regionArg = command.imgTopLeftX .. ":" .. command.imgTopLeftY .. ":" .. command.imgBottomRightX .. ":" .. command.imgBottomRightY
    local outputArg = "\"" .. command.outputDir .. "/" .. command.namespace .. command.id .. ".png" .. "\""
    local commandArg = inkscapeCmd .. " -z -a=" .. regionArg .. " -w=" .. command.textureWidth .. " -h=" .. command.textureHeight .. " -e=" .. outputArg .. " -f=" .. command.svgFilePath
    print(commandArg)
    os.execute(commandArg)
end