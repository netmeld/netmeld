DESCRIPTION
===========

This provides a library containing the core functionality for the module to
leverage.  Common functions and capabilities are pooled into this library to
enable consistent behaviour for module tools and quicker tool development.

HANDLER FUNDAMENTALS
====================

GIT
---
This handler leverages `git` as the data lake backend.  While manual
manipulation of the repository can be performed, it may cause unexpected
behaviour.

One case is for determining the ingest tool and its arguments.  Specifically,
this handler performs a regex search of the git commit body for the following
lines:
* `^ingest-tool:(.*)$`
* `^tool-args:(.*)$`

Whatever those regex values match on will be what is used as the ingest tool
and its argument(s), if any.  Outside of that, the commit message can contain
any extra data needed.

Another case is the removal of data.  The logic will, at least, search and
remove data from where it knows about.  However if the repository is manually
manipulated it may miss data to purge.  Also, the removal can be
destructive of manually added/modified data in certain cases because the entire
git commit history may be re-written.
