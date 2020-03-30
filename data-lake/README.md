DESCRIPTION
===========

Description of what the tool parses.

Various information about the tool.  Include any special considerations when
using the tool.  Potentially explain option requirements more in depth.



OTHER
=====
Layout sample with remarks:
```
~/.netmeld/datalake/ (git storage mechanism?)
|> by-device-id (binned primary storage?)
|-> device01
|--> config.txt (filename when added)
|--> doc.txt
|--> pic.png
|--> scan.nessus
|> by-tool (links only? ref to data in by-device-id?)
|-> nmdb-import-cisco
|--> device01--config.txt (device-id--filename?)
|-> nmdb-import-nessus
|--> device01--scan.nessus
|> unbinned (link to data in by-device-id, but not in by-tool?)
|--> device01--doc.txt
|--> device01--pic.png
```
nmdl-initialize
toolname --type data-lake-type

nmdl-add (merge with update? have to distinguish between add new and update existing...)
nmdl-update
  toolname somefile --device-id device
  toolname somefile --device-id device --tool nmdb-import-cisco
  toolname repofile --device-id device --tool nmdb-import-cisco-nxos
  toolname somefile --device-id device --tool nmdb-import-cisco --tool-args 'some args'
  toolname repofile --device-id device --tool nmdb-import-cisco-nxos --tool-args 'some args'
  toolname repofile --device-id device --tool-args 'some args'
  toolname repofile --device-id device --new somefile
nmdl-remove
  toolname repofile --device-id device
  toolname repofile --device-id device --permanent
nmdl-list
  toolname --by-device-id
  toolname --by-tool
  toolname --unbinned
nmdl-import-script
  toolname
  toolname --dbname dbname
  toolname --from-dts yyyymmdd
