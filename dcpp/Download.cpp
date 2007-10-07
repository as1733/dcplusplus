/*
 * Copyright (C) 2001-2007 Jacek Sieka, arnetheduck on gmail point com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "Download.h"

#include "UserConnection.h"
#include "QueueItem.h"
#include "SharedFile.h"
#include "HashManager.h"

namespace dcpp {

Download::Download(UserConnection& conn, const string& pfsDir) throw() : Transfer(conn), 
	path(pfsDir), file(0), crcCalc(NULL), treeValid(false) 
{
	conn.setDownload(this);
	setType(TYPE_PARTIAL_LIST);
}

Download::Download(UserConnection& conn, QueueItem& qi, bool supportsTrees) throw() : Transfer(conn),
	path(qi.getTarget()), tempTarget(qi.getTempTarget()), file(0),
	crcCalc(NULL), treeValid(false) 
{
	conn.setDownload(this);
	
	setTTH(qi.getTTH());
	
	if(qi.isSet(QueueItem::FLAG_USER_LIST)) {
		setType(TYPE_FULL_LIST);
	}

	if(qi.getSize() != -1) {
		if(HashManager::getInstance()->getTree(getTTH(), getTigerTree())) {
			setTreeValid(true);
			setSegment(qi.getNextSegment(getTigerTree().getBlockSize()));
		} else if(supportsTrees && !qi.getSource(conn.getUser())->isSet(QueueItem::Source::FLAG_NO_TREE) && qi.getSize() > HashManager::MIN_BLOCK_SIZE) {
			// Get the tree unless the file is small (for small files, we'd probably only get the root anyway)
			setType(TYPE_TREE);
			getTigerTree().setFileSize(qi.getSize());
		} else {
			// Use the root as tree to get some sort of validation at least...
			getTigerTree() = TigerTree(qi.getSize(), qi.getSize(), getTTH());
			setTreeValid(true);
			setSegment(qi.getNextSegment(getTigerTree().getBlockSize()));
		}
		
		if(qi.isSet(QueueItem::FLAG_RESUME)) {
			const string& target = (getTempTarget().empty() ? getPath() : getTempTarget());
			int64_t start = File::getSize(target);

#ifdef PORT_ME
			// Only use antifrag if we don't have a previous non-antifrag part
			if( BOOLSETTING(ANTI_FRAG) && (start == -1) ) {
				int64_t aSize = File::getSize(target + Download::ANTI_FRAG_EXT);

				if(aSize == d->getTotal()) {
					start = d->getStartPos();
				} else {
					start = 0;
				}
				d->setFlag(Download::FLAG_ANTI_FRAG);
			}
#else
			setFlag(Download::FLAG_ANTI_FRAG);
#endif
		}
	}
	
}

Download::~Download() {
	getUserConnection().setDownload(0);
}

AdcCommand Download::getCommand(bool zlib) {
	AdcCommand cmd(AdcCommand::CMD_GET);
	
	cmd.addParam(Transfer::names[getType()]);

	if(getType() == TYPE_PARTIAL_LIST || getType() == TYPE_FULL_LIST) {
		cmd.addParam(Util::toAdcFile(getPath()));
	} else {
		cmd.addParam("TTH/" + getTTH().toBase32());
	}
	
	cmd.addParam(Util::toString(getPos()));
	cmd.addParam(Util::toString(getSize() - getPos()));

	if(zlib && BOOLSETTING(COMPRESS_TRANSFERS)) {
		cmd.addParam("ZL1");
	}

	return cmd;
}

void Download::getParams(const UserConnection& aSource, StringMap& params) {
	Transfer::getParams(aSource, params);
	params["target"] = getPath();
	params["sfv"] = Util::toString(isSet(Download::FLAG_CRC32_OK) ? 1 : 0);
}

void Download::setSharedFile(SharedFile* f)  { 
	file = sharedFile = f;
}

} // namespace dcpp