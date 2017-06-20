/*
 *      Copyright (C) 2010-2016 Hendrik Leppkes
 *      http://www.1f0.de
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#pragma once

#include <string>
#include <list>

class CCueSheet
{
public:
  CCueSheet();
  ~CCueSheet();

  HRESULT Parse(std::string cueSheet);

public:
  struct Track {
    int index;
    std::string Id;
    std::string Title;
    REFERENCE_TIME Time;
    std::string Performer;
  };

  std::string m_Performer;
  std::string m_Title;
  std::list<Track> m_Tracks;

  std::string FormatTrack(Track & track);
};
