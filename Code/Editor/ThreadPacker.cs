using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Drawing;

namespace Editor
{
    public class ThreadPacker
    {
        List<Bitmap> bitmaps;
        int size;
        int elementWidth;
        string filename;

        public ThreadPacker(List<Bitmap> maps, int elemWidth, int atalsSize, string filepath)
        {
            bitmaps = maps;
            size = atalsSize;
            elementWidth = elemWidth;
            filename = filepath;
        }

        public void ThreadProc()
        {
            Bitmap resultBitmap = new Bitmap(size, size);
            int index = 0;
            int numCol = size / elementWidth;
            foreach (Bitmap bitmap in bitmaps)
            {
                int curRow = index / numCol;
                int curCol = index % numCol;

                int startX = curCol * elementWidth;
                int startY = curRow * elementWidth;
                for (int y = 0; y < elementWidth; y++)
                {
                    for (int x = 0; x < elementWidth; x++)
                    {
                        resultBitmap.SetPixel(startX + x, startY + y, bitmap.GetPixel(elementWidth - y - 1, x));
                    }
                }
                ++index;
            }
            resultBitmap.Save(filename);
        }
        
    }
}
