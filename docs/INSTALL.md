# Introduction

These instructions assume that you are installing Netmeld onto either Kali
rolling or Debian testing.  If you are using a different distribution, package
names and availability may differ. 

# Install the Dependencies
The following breaks down the dependencies based on function.  This can be
split even more if only targeted toolsets or version lock are needed
(e.g., see the `deb` creation scripts), however only the general developer case
is covered.  As `root`:
```
apt update

# Needed for complete toolset build
apt install \
  build-essential cmake \
  libpqxx-dev \
  libboost-date-time-dev libboost-iostreams-dev \
    libboost-program-options-dev libboost-system-dev \
    libboost-test-dev libboost-regex-dev \
  libpugixml-dev \
  libpcap0.8-dev

# Needed for complete toolset execution
apt install \
  postgresql postgresql-client \
  arping nmap macchanger wireshark-common \
  ansible \
  git

# Useful, but not technically needed
apt install \
  help2man pandoc \
  graphviz \
  postgresql-contrib postgresql-autodoc
```


# Build and Install the Netmeld Software
Ensure you have cloned the `netmeld` library and are in the `netmeld` directory. 

* Create Makefiles
	`cmake -S . -B ./build`
* Building Source
	`cmake --build ./build`
  * Building Tests (example)
	`cmake --build ./build --target Test.netmeld`
  * Running Tests (example)
	`cmake --build ./build --target test`
* Installing
	`sudo cmake --install build`


# Running the toolset
On fresh, bare installs you may have to run `sudo ldconfig` to configure the
dynamic linker run time bindings.


## Add a datastore admin/superuser
The tools will attempt to add data to the data store as the current user.
While you can drastically limit permissions, the simple case of adding the
current user as a superuser will be covered.  In this case, we will add `root`
as a superuser.

```
su postgres
createuser -s root
exit
```


## Enable and Start the Postgres Database Service
Under the assumption we always want the data store to be available to interact
with the toolset and it is co-located with the tools.

```
systemctl enable postgresql
systemctl restart postgresql
```


## Create the Netmeld Database
The default database is named `site`.

```
nmdb-initialize
```

To create a non-default database, `nmdb-initialize --db-name [dbname]`.

To create a visualization to assist in understanding the database schema,
assuming using the default database name of `site`:
```
postgresql_autodoc -d site -t dot
dot -Granksep=3.0 -Gnodesep=0.5 -Tpng -o site.png site.dot
```
