// nPDF main function
// Copyright (C) 2014  Legimet
//
// This file is part of nPDF.
//
// nPDF is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// nPDF is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with nPDF.  If not, see <http://www.gnu.org/licenses/>.

extern "C" {
#include <mupdf/fitz.h>
}
#include <cstdlib>
#include <keys.h>
#include <libndls.h>
#include <string>
#include "Viewer.hpp"
#include "Screen.hpp"
#include "Timer.hpp"

// Actions that, when held down, result in a long delay initially and a short delay afterwards
enum ScrollAction {
    none = 0,
    down = 1,
    up = 2,
    right = 4,
    left = 8,
    pgdown = 16,
    pgup = 32,
    zoomout = 64,
    zoomin = 128
};

ScrollAction operator|(ScrollAction a, ScrollAction b) {
    return static_cast<ScrollAction>(static_cast<int>(a) | static_cast<int>(b));
}

ScrollAction& operator|=(ScrollAction& a, ScrollAction b) {
    return (a = a | b);
}

const int longDelay = 400;
const int shortDelay = 40;

ScrollAction getScrollKey() {
    ScrollAction action=none;
    if (isKeyPressed(KEY_NSPIRE_DOWN) || isKeyPressed(KEY_NSPIRE_RIGHTDOWN) || isKeyPressed(KEY_NSPIRE_DOWNLEFT) ||
	    isKeyPressed(KEY_NSPIRE_2) || isKeyPressed(KEY_NSPIRE_3) || isKeyPressed(KEY_NSPIRE_1))
	action |= down;
    if (isKeyPressed(KEY_NSPIRE_UP) || isKeyPressed(KEY_NSPIRE_UPRIGHT) || isKeyPressed(KEY_NSPIRE_LEFTUP) ||
	    isKeyPressed(KEY_NSPIRE_8) || isKeyPressed(KEY_NSPIRE_7) || isKeyPressed(KEY_NSPIRE_9))
	action |= up;
    if (isKeyPressed(KEY_NSPIRE_RIGHT) || isKeyPressed(KEY_NSPIRE_UPRIGHT) || isKeyPressed(KEY_NSPIRE_RIGHTDOWN) ||
	    isKeyPressed(KEY_NSPIRE_6) || isKeyPressed(KEY_NSPIRE_9) || isKeyPressed(KEY_NSPIRE_3))
	action |= right;
    if (isKeyPressed(KEY_NSPIRE_LEFT) || isKeyPressed(KEY_NSPIRE_DOWNLEFT) || isKeyPressed(KEY_NSPIRE_LEFTUP) ||
	    isKeyPressed(KEY_NSPIRE_4) || isKeyPressed(KEY_NSPIRE_7) || isKeyPressed(KEY_NSPIRE_1))
	action |= left;
    if (isKeyPressed(KEY_NSPIRE_PLUS))
	action |= pgdown;
    if (isKeyPressed(KEY_NSPIRE_MINUS))
	action |= pgup;
    if (isKeyPressed(KEY_NSPIRE_DIVIDE))
	action |= zoomout;
    if (isKeyPressed(KEY_NSPIRE_MULTIPLY))
	action |= zoomin;
    return action;
}

void handleDelays(ScrollAction key, ScrollAction& lastScrollKey) {
    if (lastScrollKey != key) {
	Timer::start(longDelay);
	lastScrollKey = key;
    } else if (Timer::done()) {
	Timer::start(shortDelay);
    }
}

int main(int argc, char **argv) {
    Viewer v;

    if (argc >= 2) {
	v.init();
	v.openDoc(argv[1]);
    } else if (argc >= 1) {
	std::string s = std::string(argv[0]);
	size_t pos = s.find_last_of("/");
	if (pos != std::string::npos) {
	    s.erase(0, s.find_last_of("/") + 1);
	}
	if (s.size() >= 4 && s.substr(s.size() - 4) == ".tns") {
	    s.erase(s.size() - 4);
	}
	cfg_register_fileext("pdf", s.c_str());
	cfg_register_fileext("xps", s.c_str());
	cfg_register_fileext("cbz", s.c_str());
	show_msgbox("nPDF", "File extensions registered. You can now open a .pdf, .xps, or .cbz file from the Documents screen");
	return 0;
    }

    Screen::init();
    Timer::init();
    v.drawPage();
    v.display();

    ScrollAction lastScrollKey = none;
    ScrollAction current = none;

    int page;
    bool toRefresh;
    while (true) {
	if ((current = getScrollKey())) {
	    if (current != lastScrollKey || Timer::done()) {
		toRefresh = false;
		if (current & down) {
		    v.scrollDown();
		    toRefresh = 1;
		} else if (current & up) {
		    v.scrollUp();
		    toRefresh = 1;
		}
		if (current & right) {
		    v.scrollRight();
		    toRefresh = 1;
		} else if (current & left) {
		    v.scrollLeft();
		    toRefresh = 1;
		}
		if (current & pgdown) {
		    v.next();
		    toRefresh = 1;
		} else if (current & pgup) {
		    v.prev();
		    toRefresh = 1;
		}
		if (current & zoomout) {
		    v.zoomOut();
		    toRefresh = 1;
		} else if (current & zoomin) {
		    v.zoomIn();
		    toRefresh = 1;
		}
		if (toRefresh) {
		    handleDelays(current, lastScrollKey);
		    v.display();
		}
	    }
	} else {
	    lastScrollKey = none;
	    Timer::stop();
	    if (isKeyPressed(KEY_NSPIRE_ESC)) {
		break;
	    }
	    if (isKeyPressed(KEY_NSPIRE_CTRL) && isKeyPressed(KEY_NSPIRE_G)) {
		if (show_1numeric_input("Go to page", "", "Enter page number", &page, 1, v.getPages())) {
		    v.gotoPage(page - 1);
		    v.display();
		}
	    }
	}
	sleep(10);
    }

    return 0;
}
