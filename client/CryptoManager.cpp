/* 
 * Copyright (C) 2001 Jacek Sieka, jacek@creatio.se
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

#include "stdafx.h"
#include "DCPlusPlus.h"

#include "BitInputStream.h"
#include "BitOutputStream.h"

#include "CryptoManager.h"

CryptoManager* CryptoManager::instance;

string CryptoManager::keySubst(string aKey, int n) {
	BYTE* temp = new BYTE[aKey.length() + n * 10];
	
	int j=0;
	
	for(int i = 0; i<aKey.length(); i++) {
		if(isExtra(aKey[i])) {
			temp[j++] = '/'; temp[j++] = '%'; temp[j++] = 'D';
			temp[j++] = 'C'; temp[j++] = 'N';
			switch(aKey[i]) {
			case 0: temp[j++] = '0'; temp[j++] = '0'; temp[j++] = '0'; break;
			case 5: temp[j++] = '0'; temp[j++] = '0'; temp[j++] = '5'; break;
			case 36: temp[j++] = '0'; temp[j++] = '3'; temp[j++] = '6'; break;
			case 96: temp[j++] = '0'; temp[j++] = '9'; temp[j++] = '6'; break;
			case 124: temp[j++] = '1'; temp[j++] = '2'; temp[j++] = '4'; break;
			case 126: temp[j++] = '1'; temp[j++] = '2'; temp[j++] = '6'; break;
			}
			temp[j++] = '%'; temp[j++] = '/';
		} else {
			temp[j++] = aKey[i];
		}
	}
	string tmp((char*)temp, j);
	delete temp;
	return tmp;
}

string CryptoManager::makeKey(const string& lock) {
	BYTE* temp = new BYTE[lock.length()];
	int v1, v2, v3, v4, v5, v6;
	int extra=0;
	
	v1 = lock[0];
	v2 = v1^5;
	v3 = v2 / 0x10;
	v4 = v2 * 0x10;
	v5 = v4 % 0x100;
	v6 = v3 | v5;
	
	temp[0] = (BYTE)v6;
	
	for(int i = 1; i<lock.length(); i++) {
		v1 = lock[i];
		v2 = v1^lock[i-1];
		v3 = v2 / 0x10;
		v4 = v2 * 0x10;
		v5 = v4 % 0x100;
		v6 = v3 | v5;
		temp[i] = (BYTE)v6;
		if(isExtra(temp[i]))
			extra++;
	}
	
	temp[0] = (BYTE)(temp[0] ^ temp[lock.length()-1]);
	
	if(isExtra(temp[0])) {
		extra++;
	}
	
	string tmp((char*)temp, i);
	delete temp;
	return keySubst(tmp, extra);
}

void CryptoManager::decodeHuffman(const string& is, string& os) {
//	BitInputStream bis;
	int pos = 0;
	
	if(is[pos++] != 'H' || is[pos++] != 'E' || is[pos++] != '3') {
		return;
	}
	pos++;
	pos++;

	int size;
	size = *(int*)&is[pos];

	pos+=4;

	dcdebug("Size: %d\n", size);
	
	short treeSize;
	treeSize = *(short*)&is[pos];

	pos+=2;

	Leaf** leaves = new Leaf*[treeSize];
	
	for(int i=0; i<treeSize; i++) {
		int chr =  is[pos++];
		int bits = is[pos++];
		leaves[i] = new Leaf(chr, bits);
	}

	BitInputStream bis(is, pos);

	DecNode* root = new DecNode();

	for(i=0; i<treeSize; i++) {
		DecNode* node = root;
		for(int j=0; j<leaves[i]->len; j++) {
			if(bis.get()) {
				if(node->right == NULL)
					node->right = new DecNode();

				node = node->right;
			} else {
				if(node->left == NULL)
					node->left = new DecNode();

				node = node->left;
			}
		}
		node->chr = leaves[i]->chr;
	}
	
	bis.skipToByte();

	os.reserve(size+1);
	for(i=0; i<size; i++) {
		DecNode* node = root;
		while(node->chr == -1) {
			if(bis.get()) {
				node = node->right;
			} else {
				node = node->left;
			}

			if(node == NULL) {
				dcdebug("Bad node found!!!\n");
				return;
			}
		}
		os+= (BYTE)node->chr;
	}
	os[i] = 0;

	delete[] leaves;
	delete root;
}

/**
 * Counts the occurances of each characters, and adds the total number of
 * different characters to the end of the array.
 */
int CryptoManager::countChars(const string& aString, int* c, BYTE& csum) {
	int chars = 0;

	for(int i=0; i<aString.length(); i++) {

		if(c[(BYTE)aString[i]] == 0)
			chars++;

		c[(BYTE)aString[i]]++;
		csum^=(BYTE)aString[i];
	}
	return chars;
}

void CryptoManager::walkTree(list<Node*>& aTree) {
	while(aTree.size() > 1) {
		// Merge the first two nodes
		Node* node = new Node(aTree.front(), *(++aTree.begin()));
		aTree.pop_front();
		aTree.pop_front();

		bool done = false;
		for(list<Node*>::iterator i=aTree.begin(); i != aTree.end(); ++i) {
			if(*node <= *(*i)) {
				aTree.insert(i, node);
				done = true;
				break;
			}
		}

		if(!done)
			aTree.push_back(node);

	}
}

/**
 * @todo Make more effective in terms of memory allocations and copies...
 */
void CryptoManager::recurseLookup(vector<bool>* table, Node* node, vector<bool>& bytes) {
	if(node->chr != -1) {
		table[node->chr] = bytes;
		return;
	}

	vector<bool> left = bytes;
	vector<bool> right = bytes;
	
	left.push_back(false);
	right.push_back(true);

	recurseLookup(table, node->left, left);
	recurseLookup(table, node->right, right);
}

/**
 * Builds a hash table over the characters available (for fast lookup).
 * Stores each character as a set of bytes with values {0, 1}.
 */
void CryptoManager::buildLookup(vector<bool>* table, Node* aRoot) {
	vector<bool> left;
	vector<bool> right;

	left.push_back(false);
	right.push_back(true);

	recurseLookup(table, aRoot->left, left);
	recurseLookup(table, aRoot->right, right);
}

/**
 * Encodes a set of data with DC's version of huffman encoding..
 * Uses a byte[] in structure to avoid those strange iostreams where available() doesn't return the whole data set,
 * as we'll be passing multiple times over the data...
 * @todo Use real streams maybe? or something else than string (operator[] contains a compare, slow...)
 */

void CryptoManager::encodeHuffman(const string& is, string& os) {

	// First, we count all characters
	BYTE csum = 0;
	int count[256];
	memset(count, 0, sizeof(count));
	int chars = countChars(is, count, csum);

	// Next, we create a set of nodes and add it to a list, removing all characters that never occur.
	
	list<Node*> nodes;

	for(int i=0; i<256; i++) {
		if(count[i] > 0) {
			nodes.push_back(new Node(i, count[i]));
		}
	}

	nodes.sort(greater<Node*>());
	dcdebug("\n");
	for(list<Node*>::iterator it = nodes.begin(); it != nodes.end(); ++it) dcdebug("%.02x:%d, ", (*it)->chr, (*it)->weight);
	walkTree(nodes);
	dcassert(nodes.size() == 1);

	Node* root = nodes.front();
	vector<bool> lookup[256];
	
	// Build a lookup table for fast character lookups
	buildLookup(lookup, root);
	delete root;

	os.append("HE3\x0d");

	// Checksum
	os.append(1, csum);
	string::size_type sz = is.size();
	os.append((char*)&sz, 4);

	// Character count
	os.append((char*)&chars, 2);

	// The characters and their bitlengths
	for(i=0; i<256; i++) {
		if(count[i] > 0) {
			os.append(1, (BYTE)i);
			os.append(1, (BYTE)lookup[i].size());
		}
	}
	
	BitOutputStream bos(os);
	// The tree itself, ie the bits of each character
	for(i=0; i<256; i++) {
		if(count[i] > 0) {
			bos.put(lookup[i]);
		}
	}
	
	dcdebug("\nBytes: %d", os.size());
	bos.skipToByte();

	for(i=0; i<is.size(); i++) {
		dcassert(lookup[(BYTE)is[i]].size != 0);
		bos.put(lookup[(BYTE)is[i]]);
	}
}

/**
 * @file CryptoManager.cpp
 * $Id: CryptoManager.cpp,v 1.3 2001/12/01 17:15:03 arnetheduck Exp $
 * @if LOG
 * $Log: CryptoManager.cpp,v $
 * Revision 1.3  2001/12/01 17:15:03  arnetheduck
 * Added a crappy version of huffman encoding, and some other minor changes...
 *
 * Revision 1.2  2001/11/26 23:40:36  arnetheduck
 * Downloads!! Now downloads are possible, although the implementation is
 * likely to change in the future...more UI work (splitters...) and some bug
 * fixes. Only user file listings are downloadable, but at least it's something...
 *
 * Revision 1.1  2001/11/25 22:06:25  arnetheduck
 * Finally downloading is working! There are now a few quirks and bugs to be fixed
 * but what the heck....!
 *
 * @endif
 */
