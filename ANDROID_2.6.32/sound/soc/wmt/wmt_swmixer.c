/*++ 
 * linux/sound/soc/wmt/wmt_swmixer.c
 * WonderMedia I2S audio driver for ALSA
 *
 * Copyright c 2010  WonderMedia  Technologies, Inc.
 *
 * This program is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation, either version 2 of the License, or 
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details. 
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * WonderMedia Technologies, Inc.
 * 4F, 533, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C
--*/

 
#include "wmt_swmixer.h"
#include <sound/asound.h>




void wmt_pcm_fmt_trans(int fmt, int channel, char *src_buf, char *dst_buf, unsigned int chunksize)
{
	unsigned int index = 0;
	float_data_t f_data;
	unsigned short data;

	if ((fmt == SNDRV_PCM_FORMAT_FLOAT) && (channel == 2)) {
		/* transfer from 2ch float(8 bytes) to 2ch s16le(4 bytes) */
		for (index = 0; index < (chunksize / 4); ++index) {
			f_data = *((float_data_t *)src_buf + index);
			
			if (!f_data.sign) {
				data = (f_data.frac + 0x800000) >> (8 - (f_data.exp - 127));
			}
			else {
				data = ~((f_data.frac + 0x800000) >> (8 - (f_data.exp - 127))) + 1;
			}

			*((unsigned short *)dst_buf + index) = data;
		}
	}
	else if ((fmt == SNDRV_PCM_FORMAT_FLOAT) && (channel == 1)) {
		/* transfer from 1ch float(4 bytes) to 2ch s16le(4 bytes) */
		for (index = 0; index < (chunksize / 4); ++index) {
			f_data = *((float_data_t *)src_buf + index);
			
			if (!f_data.sign) {
				data = (unsigned short)((f_data.frac + 0x800000) >> (8 - (f_data.exp - 127)));
			}
			else {
				data = (unsigned short)(~((f_data.frac + 0x800000) >> (8 - (f_data.exp - 127))) + 1);
			}

			*((unsigned int *)dst_buf + index) = (data << 16) | data;
		}
	}	
}

