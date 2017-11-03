# Introduction

These instructions assume that you are installing Netmeld onto either
Kali 2.0, Debian stable/testing (currently 9/10).  If you are using a different distribution,
package names and availability may differ.  It has also been reported as
successfully installing and runnable on Ubuntu 16.04.

Netmeld depends on features in PostgreSQL 9.2 or later.
Kali 2.0 and Debian 8.x both include PostgreSQL 9.4, meeting this requirement.
If your system has an older version of PostgreSQL (such as 9.1),
you will need to upgrade to PostgreSQL 9.2 or later.


# Install the Dependencies

```
aptitude update
aptitude safe-upgrade

# Required to build
aptitude install \
    build-essential cmake make gcc g++ help2man \
    libboost-all-dev libboost-dev libpqxx-dev libpugixml-dev libpcap0.8-dev
    
# Required to run
aptitude install \
    postgresql postgresql-client postgresql-contrib postgresql-autodoc \
    python python-psycopg2 python-ipaddr python-setuptools \
    python3 python3-psycopg2 python3-setuptools \
    graphviz nmap
```

There are not yet Debian/Kali packages for the ciscoconfparse Python library.
Download and install the ciscoconfparse library from
https://pypi.python.org/pypi/ciscoconfparse. Ensure you follow install 
procedures for the version mapped to `python` (probably version 2.7 by default).


# Make `root` a DB admin/superuser

```
su postgres
createuser -s root
exit
```


# Build and Install the Netmeld Software

```
cd netmeld
mkdir build
cd build
cmake ..
make
make test
make install
```
Note: On fresh bare installs, you may have to run `ldconfig` to configure the
dynamic linker run time bindings.

# Enable and Start the Postgres Database Service

```
systemctl enable postgresql
systemctl restart postgresql
```

# Create the Netmeld Database

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