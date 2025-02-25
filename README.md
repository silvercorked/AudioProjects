# AudioProjects

A repository storing a visual studio solution with multiple audio-related projects.

## Audio Engine
The Audio Engine is the basis for all current and future projects. It uses FMOD to do much of the audio processing.
It is built into a DLL which is included into projects.
It has minimal features at the moment, but I will be adding more later.

## Personal Music Player
A personal command-line music player.
Goals: 
	- read from json file with paths to sounds/music in folders and as individual files.
	- load all sounds (can change later to lower memory usage, ie, only load current and next song)
	- command-line key controls to change song (windows only, as that's what I have to test with)
