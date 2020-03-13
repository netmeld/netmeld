# Build and Install the Netmeld Software

The playbook tool set is a set of add-on features to Netmeld.  At a minimum
this set of tools depends on netmeld-core, netmeld-core-bin, and
playbook-core to be installed before it can be used.

# Install the Dependencies

```
aptitude update
aptitude safe-upgrade

# Required to run
aptitude install \
    arping macchanger wireshark-common
```

# Build and Install the Netmeld-Playbook Software

```
cd netmeld-playbook
mkdir build
cd build
cmake ..
make
make install
cd ..
```

# Create the Netmeld-Playbook Tables and Views

Install the extra netmeld-playbook database tables and views
into an existing netmeld database.

The default database is named `site`.
```
cd netmeld-playbook/schema
make install
cd ..
```

To install to a non-default database, edit `netmeld-playbook/schema/Makefile`,
and change the `DB_NAME` variable before running `make install`.

During the `make install`, `postgres_autodoc` and `dot` are used
to generate a graphical view of the tables in the database schema.
For the default `site` database, `site.dot` and `site.png` are generated.
This visualization should assist you in understanding the database schema.
