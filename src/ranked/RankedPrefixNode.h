/* RankedPrefixNode.h
 * Copyright (C) 2016, Álvaro Alonso
 * all rights reserved.
 *
 * Class which represents a node for an auxiliar tree for prefix top k in Ranked String Dictionaries
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *
 * Contacting the authors:
 *   Álvaro Alonso:  	alvaro.alonso.isla@alumnos.uva.es
 */

#ifndef _RANKEDPREFIXNODES_H
#define _RANKEDPREFIXNODES_H

using namespace std;


class RankedPrefixNode
{
	public:
	
		RankedPrefixNode()
		{
			nodeLeft=-1;
			nodeRight=-1;
			left=0;
			right=0;
			father=-1;
		}
		
		RankedPrefixNode(uint _ptr, int _father)
		{
			father=_father;
			ptr=_ptr;
			nodeLeft=-1;
			nodeRight=-1;
			left=0;
			right=0;
		}
		
		
		void setFather(int _father) {father=_father;}
		void setPtr(size_t _ptr) {ptr=_ptr;}
		void setLeft(size_t _left) {left=_left;}
		void setRight(size_t _right) {right=_right;}
		void setLeftson(int _nodeLeft) {nodeLeft=_nodeLeft;}
		void setRightson(int _nodeRight) {nodeRight=_nodeRight;}
		
		size_t getLeft() {return left;}
		size_t getRight() {return right;}
		int getLeftSon() {return nodeLeft;}
		int getRightSon() {return nodeRight;}
		int getFather() {return father;}
		size_t getPtr() {return ptr;}
		
	protected:
		int father; //select the father node (-1 if doesn't have father)
		size_t ptr;
		size_t left, right; //the value of doing select(ptr) (with ptr of the son) in the corresponding node of the wavelet tree. select0 for left and select1 for right
		int nodeLeft, nodeRight; //the position of the sons (-1 if doesn't have that son)
};


#endif