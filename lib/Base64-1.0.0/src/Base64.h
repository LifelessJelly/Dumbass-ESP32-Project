/*
Copyright (C) 2016 Arturo Guadalupi. All right reserved.

This library is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
*/

#ifndef _BASE64_H
#define _BASE64_H

class Base64Class{
  public:
    static int encode(char *output, char *input, int inputLength);
    static int decode(char * output, char * input, int inputLength);
    static int encodedLength(int plainLength);
    static int decodedLength(char * input, int inputLength);

  private:
    static inline void fromA3ToA4(unsigned char * A4, unsigned char * A3);
    static inline void fromA4ToA3(unsigned char * A3, unsigned char * A4);
    static inline unsigned char lookupTable(char c);
};
extern Base64Class Base64;

#endif // _BASE64_H
