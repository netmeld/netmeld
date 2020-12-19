DESCRIPTION
===========

This provides the library which contains the functionality needed to interact
with the data store back-end in a consistent manner.  Though the Datastore
module contains sub-modules, this library is currently more monolithic in the
sense that it contains all functionality and does not require separate libraries
for each of the sub-modules.  This may change over time if the need arises.


LIBRARY FUNDAMENTALS
====================

UNIT TESTS
----------
Many of the components of this library have associated unit tests.  We have
made efforts to add these overtime (as we touch the components), but it is not
perfect.  Generally speaking, our efforts have been to implement unit testing
to guard against unintended functionality changes and it is expected this is
done for all *new* work.

PARSERS
-------
The parsers contained in this library are generally reusable in multiple tools
of this module.  As such, particular attention is given to abide by the formal
documentation (e.g., RFCs) for each of them and any *new* parsers for this
library must document what is the formal documentation being followed.

OBJECTS TO STORES
-----------------
Almost all of the objects in this library map to a singular store within the
data store back-end.  However, this is not always the case and some objects
contain others so there is a chaining effect in terms of data entry and
updates.  While it was considered, it is thought to create a tighter coupling
to the data store back-end as well as *lose* certain context about the data
which would otherwise be present in the current representation.

OBJECT STANDARDIZATION
----------------------
All the objects shall inherit from the `AbstractDatastoreObject` class.

While there may be very select exceptions, the following are functions all
objects in the library should implement:
* `toDebugString`:
  * Is required to be implemented and expected to be used for understanding
    the internal state of the object.  Primarily, for troubleshooting with
    end-users and aid in regression testing.
  * The output is on a singular line and does not end with a new line.
  * The output follows JSON syntax.
  * The output contains all important internal variable values.
* `isValid`:
  * Checks key information for validity within the object.
  * Returns `true` if the object is considered *valid*
    * Note, this does not necessarily mean it will be stored in the data store.
      While this may be desirable in the long term, it currently is not as
      there are instances this would create incorrect behavior and not save
      sub-object data.
* `save`:
  * Performs a validity check before saving to the data store and logs, to
    `DEBUG`, the `toDebugString` if invalid.
    * Note, this validity check should be from the perspective of the object
      is valid and it will be saved to the data store.
