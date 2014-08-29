using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Threading;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;
using Controllers;

namespace Editor
{
    public partial class Form1 : Form
    {
        private Controllers.My3DView my3DView;
        private Controllers.EngineBridge engine;

        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            my3DView = new My3DView();
            engine = new EngineBridge();
        }

        private void my3DView1_Load(object sender, EventArgs e)
        {
        }

        private void pictureBox1_Click(object sender, EventArgs e)
        {

        }

        private void button1_Click(object sender, EventArgs e)
        {
            pictureBox1.Size = new Size(256, 256);
            Bitmap bm = new Bitmap(256, 256);
            float persistence = (float)numericUpDown1.Value;
            float size = (float)numericUpDown2.Value;

            byte[,] colors = engine.GeneratePerlin(256, 256, persistence, size);
            for (int i = 0; i < 256; i++)
            {
                for (int j = 0; j < 256; j++)
                {
                    bm.SetPixel(i, j, Color.FromArgb(colors[i, j], colors[i, j], colors[i, j]));
                }
            }
            pictureBox1.Image = bm;


            Console.Write(colors[0, 0]);



        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            engine.Dispose();
            my3DView.Dispose();
        }

        private void numericUpDown1_ValueChanged(object sender, EventArgs e)
        {

        }

        private void button2_Click(object sender, EventArgs e)
        {
            var files = System.IO.Directory.EnumerateFiles(uvAnimSelectedPath.Text);
            int frames = (int)uvFrames.Value;
            System.Diagnostics.Debug.Assert(frames % 4 == 0);
            int num = 0;

            Bitmap[] bitmaps = new Bitmap[frames];
            int width = 0;
            int height = 0;
            foreach (string currentFile in files)
            {
                string filename = System.IO.Path.GetFileNameWithoutExtension(currentFile);
                bitmaps[num] = (Bitmap)Bitmap.FromFile(currentFile);
                width = bitmaps[num].Width;
                height = bitmaps[num].Height;
                ++num;
                if (num >= frames)
                    break;
            }

            int numRow = (int)Math.Sqrt(frames);
            Bitmap result = new Bitmap(numRow * width, numRow * height);
            int index = 0;
            foreach (Bitmap bitmap in bitmaps)
            {
                int curRow = index / numRow;
                int curCol = index % numRow;

                int startX = curCol * width;
                int startY = curRow * height;
                for (int y = 0; y < height; y++)
                {
                    for (int x = 0; x < width; x++)
                    {
                        result.SetPixel(startX + x, startY + y, bitmaps[index].GetPixel(x, y));
                    }
                }
                ++index;
            }
            string outFile = uvAnimSelectedPath.Text + "\\" + outFileName.Text;
            result.Save(outFile);
            MessageBox.Show("Generated.");
            System.Diagnostics.Debug.Assert(frames == num);

        }

        private void button1_Click_1(object sender, EventArgs e)
        {
            if (folderBrowserDialog1.SelectedPath.Length==0)
                folderBrowserDialog1.SelectedPath = @"D:\Projects\fastbird-engine\Data\";
            DialogResult result = folderBrowserDialog1.ShowDialog();
            if (result == DialogResult.OK)
            {
                String path = folderBrowserDialog1.SelectedPath;
                uvAnimSelectedPath.Text = path;
            }
        }

        private void label3_Click(object sender, EventArgs e)
        {

        }

        private void outputFileBtn_Click(object sender, EventArgs e)
        {
            DialogResult result = saveFileDialog1.ShowDialog();
            if (result == DialogResult.OK)
            {
                String path = saveFileDialog1.FileName;
                outFileName.Text = path;
            }
        }

        private void genWorldBtn_Click(object sender, EventArgs e)
        {
            //int XRange=320;
            //int YRange=160;
            //int[] WorldMapArray = new int[XRange * YRange];
            //float[] SinIterPhi = new float[2*XRange];
            //for(int i=0; i<XRange; ++i)
            //{
            //    SinIterPhi[i] = SinIterPhi[i+XRange] = (float)Math.Sin( i * 2.0f*Math.PI/XRange );
            //}
            //int[] Histogram = new int[256];
            //int FilledPixels;
            //int[] Red = new int[49]{0,0,0,0,0,0,0,0,34,68,102,119,136,153,170,187,
            //     0,34,34,119,187,255,238,221,204,187,170,153,
            //     136,119,85,68,
            //     255,250,245,240,235,230,225,220,215,210,205,200,
            //     195,190,185,180,175};
            //int[] Green = new int[49]{0,0,17,51,85,119,153,204,221,238,255,255,255,
            //     255,255,255,68,102,136,170,221,187,170,136,
            //     136,102,85,85,68,51,51,34,
            //     255,250,245,240,235,230,225,220,215,210,205,200,
            //     195,190,185,180,175};
            //int[] Blue = new int[49]{0,68,102,136,170,187,221,255,255,255,255,255,
            //     255,255,255,255,0,0,0,0,0,34,34,34,34,34,34,
            //     34,34,34,17,0,
            //     255,250,245,240,235,230,225,220,215,210,205,200,
            //     195,190,185,180,175};

            //uint seed = 0;
            //int NumberOfFaults = 5;
            //int PercentWater = 20;
            //int PercentIce = 20;

            //for (int j=0, row=0; j<XRange; j++)
            //{
            //    WorldMapArray[row] = 0;
            //    for (int i=1; i<YRange; i++) 
            //        WorldMapArray[i+row] = int.MinValue;
            //        row += YRange;
            //}

            //float YRangeDiv2 = YRange/2;
            //float YRangeDivPI = (float)YRange / (float)Math.PI;
            //Random rnd = new Random();
            //for (int a=0; a<NumberOfFaults; a++)
            //{
            //    float         Alpha, Beta;
            //    float         TanB;
            //    float         Result, Delta;
            //    int           i, row, N2;
            //    int           Theta, Phi, Xsi;
            //    uint  flag1;


                
            //    flag1 = (uint)rnd.Next() & 1;
  
            //    //  -0.5PI<= Alpha < 0.5PI
            //    Alpha = (((float) rnd.Next())/int.MaxValue-0.5f) * (float)Math.PI; /* Rotate around x-axis */
            //    Beta  = (((float) rnd.Next())/int.MaxValue-0.5f) * (float)Math.PI; /* Rotate around y-axis */

            //    // 0<= TanB <=PI
            //    TanB  = (float)Math.Tan(Math.Acos(Math.Cos(Alpha)*Math.Cos(Beta)));
  
            //    row  = 0;
            //    // -PI <= Xsi <=PI
            //    Xsi  = (int)( XRange / 2 - ( XRange /Math.PI ) * Beta );
  
            //    for (Phi=0; Phi<XRange/2; Phi++)
            //    {
            //        Theta = (int)((YRangeDivPI*Math.Atan(SinIterPhi[Xsi-Phi+XRange]*TanB))+YRangeDiv2);

            //        if (flag1!=0)
            //        {
            //          /* Rise northen hemisphere <=> lower southern */
            //          if (WorldMapArray[row+Theta] != int.MinValue)
            //            WorldMapArray[row+Theta]--;
            //          else
            //            WorldMapArray[row+Theta] = -1;
            //        }
            //        else
            //        {
            //          /* Rise southern hemisphere */
            //          if (WorldMapArray[row+Theta] != int.MinValue)
            //               WorldMapArray[row+Theta]++;
            //          else
            //               WorldMapArray[row+Theta] = 1;
            //        }
            //        row += YRange;
            //    }
            //}

            //int index2 = (XRange/2)*YRange;
            //for (int j=0, row=0; j<XRange/2; j++)
            //{
            //    for (int i=0; i<YRange; i++)
            //    {
            //        WorldMapArray[row+index2+YRange-i] = WorldMapArray[row+i];
            //    }
            //    row += YRange;
            //}

            //for (int j=0, row=0; j<XRange; j++)
            //{
            //    /* We have to start somewhere, and the top row was initialized to 0,
            //        * but it might have changed during the iterations... */
            //    int cr = WorldMapArray[row];
            //    for (int i=1; i<YRange; i++)
            //    {
            //        /* We "fill" all positions with values != INT_MIN with Color */
            //        int Cur = WorldMapArray[row+i];
            //        if (Cur != int.MinValue)
            //        {
            //           cr += Cur;
            //        }
            //        WorldMapArray[row+i] = cr;
            //    }
            //    row += YRange;
            //}

            //int MaxZ = 1;
            //int MinZ = -1;
            // for (int j=0; j<XRange*YRange; j++)
            //  {
            //    int cr = WorldMapArray[j];
            //    if (cr > MaxZ) MaxZ = cr;
            //    if (cr < MinZ) MinZ = cr;
            //  }

            //for (int j=0, row=0; j<XRange; j++)
            //  {
            //    for (int i=0; i<YRange; i++)
            //    {
            //      int cr = WorldMapArray[row+i];
            //      cr = (int)(((float)(cr - MinZ + 1) / (float)(MaxZ-MinZ+1))*30)+1;
            //      Histogram[cr]++;
            //    }
            //    row += YRange;
            //  }

            //int Threshold = PercentWater * XRange * YRange / 100;
            //int jbackup=0;
            //for (int j = 0, Count = 0; j < 256; j++)
            //{
            //    Count += Histogram[j];
            //    if (Count > Threshold)
            //    {
            //        jbackup = j;
            //        break;
            //    }
            //}

            //Threshold = jbackup * (MaxZ - MinZ + 1) / 30 + MinZ;

            //int TwoColorMode = 0;

            //if (TwoColorMode!=0)
            //  {
            //    for (int j=0, row=0; j<XRange; j++)
            //    {
            //      for (int i=0; i<YRange; i++)
            //      {
            //        int cr = WorldMapArray[row+i];
            //        if (cr < Threshold)
            //          WorldMapArray[row+i] = 3;
            //        else
            //          WorldMapArray[row+i] = 20;
            //      }
            //      row += YRange;
            //    }
            //  }
            //  else
            //  {
            //    /* Scale WorldMapArray to colorrange in a way that gives you
            //     * a certain Ocean/Land ratio */
            //    for (int j=0, row=0; j<XRange; j++)
            //    {
            //      for (int i=0; i<YRange; i++)
            //      {
            //        int cr = WorldMapArray[row+i];
	
            //        if (cr < Threshold)
            //            cr = (int)(((float)(cr - MinZ) / (float)(Threshold - MinZ))*15)+1;
            //        else
            //            cr = (int)(((float)(cr - Threshold) / (float)(MaxZ - Threshold))*15)+16;
	
            //    /* Just in case... I DON't want the GIF-saver to flip out! :) */
            //    if (Color < 1) Color=1;
            //    if (Color > 255) Color=31;
            //    WorldMapArray[row+i] = Color;
            //      }
            //      row += YRange;
            //    }

            //    /* "Recycle" Threshold variable, and, eh, the variable still has something
            //     * like the same meaning... :) */
            //    Threshold = PercentIce*XRange*YRange/100;

            //    if ((Threshold <= 0) || (Threshold > XRange*YRange)) goto Finished;

            //    FilledPixels = 0;
            //    /* i==y, j==x */
            //    for (i=0; i<YRange; i++)
            //    {
            //      for (j=0, row=0; j<XRange; j++)
            //      {
            //        Color = WorldMapArray[row+i];
            //        if (Color < 32) FloodFill4(j,i,Color);
            //        /* FilledPixels is a global variable which FloodFill4 modifies...
            //             * I know it's ugly, but as it is now, this is a hack! :)
            //             */
            //        if (FilledPixels > Threshold) goto NorthPoleFinished;
            //            row += YRange;
            //      }
            //    }
    
            //NorthPoleFinished:
            //    FilledPixels=0;
            //    /* i==y, j==x */
            //    for (i=YRange; i>0; i--)
            //    {
            //      for (j=0, row=0; j<XRange; j++)
            //      {
            //    Color = WorldMapArray[row+i];
            //    if (Color < 32) FloodFill4(j,i,Color);
            //    /* FilledPixels is a global variable which FloodFill4 modifies...
            //         * I know it's ugly, but as it is now, this is a hack! :)
            //         */
            //    if (FilledPixels > Threshold) goto Finished;
            //        row += YRange;
            //      }
            //    }
            //Finished:
            //  }

        }

        private void btnGenModulePack_Click(object sender, EventArgs e)
        {
            var dirs = System.IO.Directory.EnumerateDirectories(uvAnimSelectedPath.Text);
            int frames = (int)uvFrames.Value;
            int num = 0;

            List<Bitmap> diffuses = new List<Bitmap>();
            List<Bitmap> metalics = new List<Bitmap>();
            List<Bitmap> normals = new List<Bitmap>();
            List<Bitmap> roughness = new List<Bitmap>();
            foreach (string currentDir in dirs)
            {
                bool match = System.Text.RegularExpressions.Regex.IsMatch(currentDir, "_texture$");
                if (!match)
                    continue;
                var files = System.IO.Directory.EnumerateFiles(currentDir + "\\Material_material");
                foreach(string currentFile in files)
                {
                    string filename = System.IO.Path.GetFileName(currentFile);
                    if (filename == "diffuse.png")
                    {
                        diffuses.Add((Bitmap)Bitmap.FromFile(currentFile));
                    }
                    else if (filename == "metallic.png")
                    {
                        metalics.Add((Bitmap)Bitmap.FromFile(currentFile));
                    }
                    else if (filename == "normal.png")
                    {
                        normals.Add((Bitmap)Bitmap.FromFile(currentFile));
                    }
                    else if (filename == "roughness.png")
                    {
                        roughness.Add((Bitmap)Bitmap.FromFile(currentFile));
                    }
                    ++num;
                }
                
            }
            ThreadPacker diffusePacker = new ThreadPacker(diffuses, 512, 2048, uvAnimSelectedPath.Text + "\\diffusepack.png");
            Thread diffuseThread = new Thread(new ThreadStart(diffusePacker.ThreadProc));
            diffuseThread.Start();

            ThreadPacker metalicPacker = new ThreadPacker(metalics, 512, 2048, uvAnimSelectedPath.Text + "\\metalicpack.png");
            Thread metalicThread = new Thread(new ThreadStart(metalicPacker.ThreadProc));
            metalicThread.Start();

            ThreadPacker normalPacker = new ThreadPacker(normals, 512, 2048, uvAnimSelectedPath.Text + "\\normalpack.png");
            Thread normalThread = new Thread(new ThreadStart(normalPacker.ThreadProc));
            normalThread.Start();

            ThreadPacker roughnessPacker = new ThreadPacker(roughness, 512, 2048, uvAnimSelectedPath.Text + "\\roughnesspack.png");
            Thread roughnessThread = new Thread(new ThreadStart(roughnessPacker.ThreadProc));
            roughnessThread.Start();

            diffuseThread.Join();
            metalicThread.Join();
            normalThread.Join();
            roughnessThread.Join();

            roughness.Clear();
            normals.Clear();
            diffuses.Clear();
            metalics.Clear();
        }

    }
}
