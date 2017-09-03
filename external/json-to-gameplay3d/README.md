[![Build Status](https://travis-ci.org/louis-mclaughlin/gameplay-json-properties.svg?branch=master)](https://travis-ci.org/louis-mclaughlin/json-properties)
==============
```
$ mkdir build
$ cd build
$ cmake ..
$ make
$ ./json2gp3d -i example.json -o example.gp
```
```json
{"frames": [{
	"filename": "run1.png",
	"frame": {"x":0,"y":0,"w":100,"h":120},
	"rotated": false,
	"trimmed": false,
	"spriteSourceSize": {"x":0,"y":0,"w":100,"h":120},
	"sourceSize": {"w":100,"h":120}
}]}
```
```
frames
{
    frames_0
    {
        filename = run1.png

        frame
        {
            h = 120
            w = 100
            x = 0
            y = 0
        }

        rotated = false

        sourceSize
        {
            h = 120
            w = 100
        }

        spriteSourceSize
        {
            h = 120
            w = 100
            x = 0
            y = 0
        }

        trimmed = false
    }
}
```
