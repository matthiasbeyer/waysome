/*
 * waysome - wayland based window manager
 *
 * Copyright in alphabetical order:
 *
 * Copyright (C) 2014-2015 Julian Ganz
 * Copyright (C) 2014-2015 Manuel Messner
 * Copyright (C) 2014-2015 Marcel Müller
 * Copyright (C) 2014-2015 Matthias Beyer
 * Copyright (C) 2014-2015 Nadja Sommerfeld
 *
 * This file is part of waysome.
 *
 * waysome is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * waysome is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with waysome. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @addtogroup action "Action manager"
 *
 * @{
 */

/**
 * @addtogroup action_commands "Action manager commands"
 *
 * @{
 */

#ifndef __WS_ACTION_COMMANDS_H__
#define __WS_ACTION_COMMANDS_H__

#include "command/command.h"

/**
 * Add commands which have impact on the processor
 *
 * @return 0 on success, a negative error number otherwise
 */
int
ws_action_commands_init(void);

#endif // __WS_ACTION_COMMANDS_H__

/**
 * @}
 */

/**
 * @}
 */

