# use launchpadlib to change bugs that have a "Fix Committed" status to "Fix Released".
# authorize as "Change non-private data" when Launchpad asks for it.

from optparse import OptionParser

parser = OptionParser(usage = '%prog version [bugs_to_exclude]')
options, args = parser.parse_args()
if len(args) < 1:
	parser.error('No version defined')
message = 'Fixed in DC++ ' + args[0] + '.'
exclude = list()
if len(args) > 1:
	exclude = [int(str) for str in args[1:]]

from launchpadlib.launchpad import Launchpad
from launchpadlib.errors import HTTPError

launchpad = Launchpad.login_with('DC++ release script', 'production')
project = launchpad.projects['dcplusplus']

bug_tasks = project.searchTasks(status = 'Fix Committed')

changed = 0
unchanged = list()
total = 0

for bug_task in bug_tasks:
	if bug_task.bug.id in exclude:
		continue
	total = total + 1
	try:
		bug_task.transitionToStatus(status = 'Fix Released')
		bug_task.bug.newMessage(content = message)
		changed = changed + 1
	except HTTPError:
		unchanged.append(bug_task.bug.id)

print '%d/%d bugs have been changed.' % (changed, total)
for id in unchanged:
	print 'Bug #%d could not be changed.' % id
