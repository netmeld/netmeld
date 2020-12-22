# Introduction

These instructions assume that we are installing Netmeld onto either Kali
rolling or Debian testing from source.  If we are using a different
distribution, package names and availability may differ but we are aware of it
working fine on several other distributions as well.  If we were wanting to
install just the tools and from the `deb` files, then see
[releases](https://github.com/netmeld/netmeld/releases).


# Build Environment Configuration
There are a couple of routes, so choose a best fit for the particular use case
needed.


## Build with Docker
We use a Docker container for our automated builds on GitHub, so that is an
option.  The container can be found on
[Docker Hub](https://hub.docker.com/r/netmeld/netmeld-dev)
and assuming we have Docker installed and can pull from `hub.docker.com`
normally, we can run the following to grab the image:
```
docker pull netmeld/netmeld-dev
```

Note that this will only get a build environment configured.  We will still
need to grab the source and get it into the Docker environment somehow (e.g.,
git clone, mount a local directory), run as the appropriate user, determine
if we want it persistent or throw away per use, etc.  Configure Docker is
beyond scope of this install guide.


## Build Dependencies
The following breaks down the dependencies based on function.  This can be
split even more if only targeted tool sets, version lock is needed, or other
various reasons.  We, however, will only cover a general developer case.
So, as a privileged user install the following:
```
apt update

# Needed for complete tool set build
apt install \
  build-essential cmake \
  libpqxx-dev \
  libboost-date-time-dev libboost-iostreams-dev \
    libboost-program-options-dev libboost-system-dev \
    libboost-test-dev libboost-regex-dev \
  libpugixml-dev \
  libpcap0.8-dev

# Needed for complete tool set execution; assumes prior installed
apt install \
  postgresql postgresql-client \
  arping nmap macchanger wireshark-common \
  ansible \
  git

# Useful, but not technically needed; assumes prior installed
apt install \
  help2man pandoc \
  graphviz \
  postgresql-contrib postgresql-autodoc
```


# Build and Install the Netmeld Software
Ensure we have cloned the `netmeld` repository and are in the `netmeld`
directory.

* Create makefiles: `cmake -S . -B ./build`
* Building source:	`cmake --build ./build`
  * Building tests (example): `cmake --build ./build --target Test.netmeld`
  * Running tests (example):	`cmake --build ./build --target test`
* Installing (as a privileged user): `cmake --install build`


# Running the Tool set
Note that on a fresh, bare install we may have to run (as a privileged user)
the `ldconfig` command to configure the dynamic linker run time bindings.

## Datalake Module Specific
These steps are needed if working on the Datalake module.

### Initialize the Data Lake
The default data lake is a `git` back-end and is, by default, located at
`~/.netmeld/datalake`.  To accept these defaults:
```
nmdl-initialize
```

If a different location is needed, `nmdl-initialize --lake-path [path]`.


## Datastore Module Specific
These steps are needed if working on the Datastore module (or associated
modules) as it currently uses a PostgreSQL database back-end and needs some
initial configuration.


### Add a Database Admin/Superuser
The tooling will attempt to add data to the database as the current user.
While we can drastically limit permissions, the simple case of adding the
current user as a superuser will be covered.  In this case, we will add `root`
as a superuser.

```
su postgres
createuser -s root
exit
```


### Enable and Start the Postgres Database Service
Under the assumption we always want the service to be available to interact
with the tool set without having to start it every time:

```
systemctl enable postgresql
systemctl restart postgresql
```


### Initialize the Data Store
The default database is named `site` and if that is fine then:

```
nmdb-initialize
```

To create a non-default database, `nmdb-initialize --db-name [dbname]`.

To create a visualization to assist in understanding the database schema with
regards to the tables.  Note, this assumes we are using the default database
name of `site` and everything outlined in the installs are installed:
```
postgresql_autodoc -d site -t dot
dot -Granksep=3.0 -Gnodesep=0.5 -Tpng -o site.png site.dot
```
