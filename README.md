# Luce

This is the source code of the [Photoshop plugin](http://amicoperry.altervista.org/luce).

The core of the plugin is the class CLuce, that is inside the luce.cpp file, It contains the source code of the plugin with all options and settings.
Other files are to interface with Photoshop.
# Other versions
## Realtime
In the subdirectory sourceRT is present the source code of the realtime version, that is essentially a version with less options.
I used it to test my CRepetitiveThread class, that can be used to create thread that do repetitively a task.
## OpenCL
In this subdirectory is present a version of in OpenCl, the code is essentially the same, adapted to use the vector types. 
It uses the [GFL SDK](http://www.xnview.com/fr/GFL/) to load the texture.

