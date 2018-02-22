/*
  Copyright 2015-2018 Giuseppe Lipari
  email: giuseppe.lipari@univ-lille.fr
  
  This file is part of TiPa.

  TiPa is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  TiPa is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
  License for more details.
  
  You should have received a copy of the GNU General Public License
  along with TiPa. If not, see <http://www.gnu.org/licenses/>
*/
#ifndef __WPTR_HPP__
#define __WPTR_HPP__

#include <memory>

namespace tipa {

    typedef enum {WPTR_WEAK, WPTR_STRONG} WPtr_type;

    /** This is a Shared/Weak pointer wrapper. It can contain either
     * a weak pointer or a shared pointer to an address. */
    template <class T>
    class WPtr {
    public: 
        WPtr(std::shared_ptr<T> addr, WPtr_type iw = WPTR_STRONG) {
            is_weak = iw;
            if (iw == WPTR_WEAK) wptr = std::weak_ptr<T>(addr);
            else sptr = std::shared_ptr<T>(addr);
        } 

        std::shared_ptr<T> get() {
            if (is_weak == WPTR_WEAK) {
                return wptr.lock();
            }
            else return sptr;
        }
        WPtr_type isWeak() const { return is_weak; }	
    private:
        WPtr_type is_weak;
        std::shared_ptr<T> sptr;
        std::weak_ptr<T> wptr;
    };
}

#endif 
