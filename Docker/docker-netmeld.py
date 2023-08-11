#!/usr/bin/python3 --

# =============================================================================
# Copyright 2022 National Technology & Engineering Solutions of Sandia, LLC
# (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
# Government retains certain rights in this software.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
# =============================================================================
# Maintained by Sandia National Laboratories <Netmeld@sandia.gov>
# =============================================================================

import argparse
import sys

def getAptCmd(cmd, passedOpts=''):
  standardOpts = ''
  standardOpts+= ' --quiet --quiet'
  standardOpts+= ' --assume-yes'
  standardOpts+= ' --option Dpkg::Use-Pty=0'
  if cmd == 'install':
    standardOpts+= ' --no-install-suggests'

  standardOpts = standardOpts.strip()

  aptCmd = f'apt {cmd} {standardOpts} {passedOpts}'

  return aptCmd

def addAptInstall(packages, opts=''):
  if packages != None and ('datastore.deb' in packages or
                           'datastore*.deb' in packages or
                           'netmeld-*.deb' in packages
                           ):
    print('RUN {0} {1} || {0} {1}'
          .format(getAptCmd('install', opts), packages))
  else:
    print('RUN {0} {1}'.format(getAptCmd('install', opts), packages))

  return 0

def createHeader():
  fromDistro = "debian:testing-slim"

  print('FROM {0}'.format(fromDistro))
  print('ENV DEBIAN_FRONTEND noninteractive')
  print('RUN useradd -m -s /bin/bash netmeld')
  print('WORKDIR /home/netmeld')

def prepInstallNetmeld():
  packages = 'curl unzip'
  curlCmd = 'curl -k --silent'
  nmWebUrl = 'https://api.github.com/repos/netmeld/netmeld/releases/latest'
  nmRegex = '\'"browser_download_url": "\K(.*)(?=")\''
  nmZipFile = 'netmeld-debs.zip'

  print('RUN {0}'.format(getAptCmd('update')))
  addAptInstall(packages)
  print('RUN {0} --location `{0} {1} | grep -oP {2}` --output {3}'
        .format(curlCmd, nmWebUrl, nmRegex, nmZipFile))
  print('RUN unzip -qq {0}'.format(nmZipFile))
  print('RUN {0} {1}'.format(getAptCmd('autoremove', '--purge'), packages))

  return 0

def cleanUp():
  print('RUN rm -rf ./netmeld*')
  print('RUN rm -rf /var/lib/apt/lists/*')

  return 0

def closeOut():
  if not config['root']:
    print('USER netmeld')

  print('CMD ["/bin/bash"]')

  return 0

def enableDbUser():
  tfile = '/etc/sudoers.d/netmeld-psql'
  tcmd  = 'echo "netmeld ALL=(root) NOPASSWD:/usr/sbin/service postgresql *"'
  print('RUN {0} > {1}'.format(tcmd, tfile))

  return 0

def buildAll():
  createHeader()
  prepInstallNetmeld()

  packages = 'sudo ./netmeld-*.deb'
  addAptInstall(packages)

  enableDbUser()

  cleanUp()
  closeOut()

  return 0

def buildClw():
  createHeader()
  prepInstallNetmeld()

  packages = './netmeld-*core.deb ./netmeld-*tool-clw.deb'
  addAptInstall(packages)

  cleanUp()
  closeOut()

  return 0

def buildDatalake():
  createHeader()
  prepInstallNetmeld()

  packages = './netmeld-*core.deb ./netmeld-*datalake.deb'
  addAptInstall(packages)

  cleanUp()
  closeOut()

  return 0

def buildDatastoreAll():
  createHeader()
  prepInstallNetmeld()

  packages = 'sudo ./netmeld-*core.deb ./netmeld-*datastore*.deb'
  addAptInstall(packages)
  enableDbUser()

  cleanUp()
  closeOut()

  return 0

def buildDatastoreDB():
  createHeader()
  prepInstallNetmeld()

  packages = 'sudo ./netmeld-*core.deb ./netmeld-*datastore.deb'
  addAptInstall(packages)
  enableDbUser()

  cleanUp()
  closeOut()

  return 0

def buildDatastoreTools():
  createHeader()
  prepInstallNetmeld()

  packages = './netmeld-*core.deb'
  addAptInstall(packages)

  packages = './netmeld-*datastore*.deb'
  addAptInstall(packages, '--no-install-recommends')

  cleanUp()
  closeOut()

  return 0

def buildDevelopment():
  createHeader()
  print('RUN {0}'.format(getAptCmd('update')))

  packages = ''
  packages+= ' debconf'
  packages+= ' build-essential'
  packages+= ' cmake'
  packages+= ' make'
  packages+= ' gcc'
  packages+= ' g++'
  packages+= ' git'
  packages+= ' help2man'
  packages+= ' pandoc'
  packages+= ' libboost-date-time-dev'
  packages+= ' libboost-iostreams-dev'
  packages+= ' libboost-program-options-dev'
  packages+= ' libboost-system-dev'
  packages+= ' libboost-test-dev'
  packages+= ' libpqxx-dev'
  packages+= ' libpugixml-dev'
  packages+= ' libpcap0.8-dev'
  packages+= ' nlohmann-json3-dev'
  packages+= ' libyaml-cpp-dev'
  packages+= ' python3'
  packages+= ' apt-transport-https'
  packages+= ' ca-certificates'
  packages = packages.strip()
  addAptInstall(packages)

  print('RUN update-ca-certificates')

  config['root'] = True

  cleanUp()
  closeOut()

  return 0

def buildFetchers():
  createHeader()
  prepInstallNetmeld()

  packages = './netmeld-*fetchers.deb'
  addAptInstall(packages)

  cleanUp()
  closeOut()

  return 0

def buildPlaybookNoDB():
  createHeader()
  prepInstallNetmeld()

  packages = './netmeld-*core.deb'
  addAptInstall(packages)

  packages = './netmeld-*datastore.deb'
  addAptInstall(packages, '--no-install-recommends')

  packages = './netmeld-*playbook.deb'
  addAptInstall(packages)

  cleanUp()
  closeOut()

  return 0

def buildTester():
  createHeader()
  prepInstallNetmeld()

  packages = 'sudo ./netmeld-*.deb'
  addAptInstall(packages)

  enableDbUser()

  cleanUp()

  print('COPY ./testing /home/netmeld/')
  print('RUN chown -R netmeld:netmeld /home/netmeld')
  print('USER netmeld')
  print('ENTRYPOINT ["/bin/bash", "/home/netmeld/run.sh"]')
  print('CMD ["run"]')

  return 0

def main():
  parser = argparse.ArgumentParser(
              description = 'Generate Dockerfile code for Netmeld tooling.'
            , epilog = 'Report bugs to <Netmeld@sandia.gov>.'
  )
  moduleChoices = [
      'all'
    , 'clw'
    , 'datalake'
    , 'datastore-all'
    , 'datastore-db'
    , 'datastore-tools'
    , 'development'
    , 'fetchers'
    , 'playbook-nodb'
    , 'tester'
  ]
  parser.add_argument('module'
                      , help='generate this module\'s Dockerfile'
                      , choices=moduleChoices
  )
  parser.add_argument('-r','--root'
                      , help='leave as root for user'
                      , action='store_true'
  )
  args = parser.parse_args()

  global config
  config = vars(args)
  val = config['module']
  if    val == 'all':
    buildAll()
  elif  val == 'clw':
    buildClw()
  elif  val == 'datalake':
    buildDatalake()
  elif  val == 'datastore-all':
    buildDatastoreAll()
  elif  val == 'datastore-db':
    buildDatastoreDB()
  elif  val == 'datastore-tools':
    buildDatastoreTools()
  elif  val == 'development':
    buildDevelopment()
  elif  val == 'fetchers':
    buildFetchers()
  elif  val == 'playbook-nodb':
    buildPlaybookNoDB()
  elif  val == 'tester':
    buildTester()
  else:
    parser.print_help()

  return 0

if __name__ == "__main__":
  rc = main()
  sys.exit(rc)
