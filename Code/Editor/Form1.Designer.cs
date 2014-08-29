using Controllers;
namespace Editor
{
    partial class Form1
    {
        /// <summary>
        /// 필수 디자이너 변수입니다.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// 사용 중인 모든 리소스를 정리합니다.
        /// </summary>
        /// <param name="disposing">관리되는 리소스를 삭제해야 하면 true이고, 그렇지 않으면 false입니다.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form 디자이너에서 생성한 코드

        /// <summary>
        /// 디자이너 지원에 필요한 메서드입니다.
        /// 이 메서드의 내용을 코드 편집기로 수정하지 마십시오.
        /// </summary>
        private void InitializeComponent()
        {
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.GeneratePerlinBtn = new System.Windows.Forms.Button();
            this.numericUpDown1 = new System.Windows.Forms.NumericUpDown();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.numericUpDown2 = new System.Windows.Forms.NumericUpDown();
            this.folderBrowserDialog1 = new System.Windows.Forms.FolderBrowserDialog();
            this.button1 = new System.Windows.Forms.Button();
            this.uvAnimSelectedPath = new System.Windows.Forms.TextBox();
            this.outFileName = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.button2 = new System.Windows.Forms.Button();
            this.uvFrames = new System.Windows.Forms.NumericUpDown();
            this.label4 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.outputFileBtn = new System.Windows.Forms.Button();
            this.saveFileDialog1 = new System.Windows.Forms.SaveFileDialog();
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.perlinHeight = new System.Windows.Forms.NumericUpDown();
            this.perlinWidth = new System.Windows.Forms.NumericUpDown();
            this.label7 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.tabPage2 = new System.Windows.Forms.TabPage();
            this.worldGenPage = new System.Windows.Forms.TabPage();
            this.worldPictureBox = new System.Windows.Forms.PictureBox();
            this.genWorldBtn = new System.Windows.Forms.Button();
            this.btnGenModulePack = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown1)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown2)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.uvFrames)).BeginInit();
            this.tabControl1.SuspendLayout();
            this.tabPage1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.perlinHeight)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.perlinWidth)).BeginInit();
            this.tabPage2.SuspendLayout();
            this.worldGenPage.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.worldPictureBox)).BeginInit();
            this.SuspendLayout();
            // 
            // pictureBox1
            // 
            this.pictureBox1.BackColor = System.Drawing.SystemColors.Control;
            this.pictureBox1.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.pictureBox1.Location = new System.Drawing.Point(8, 63);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(512, 512);
            this.pictureBox1.TabIndex = 0;
            this.pictureBox1.TabStop = false;
            this.pictureBox1.Click += new System.EventHandler(this.pictureBox1_Click);
            // 
            // GeneratePerlinBtn
            // 
            this.GeneratePerlinBtn.Location = new System.Drawing.Point(333, 36);
            this.GeneratePerlinBtn.Name = "GeneratePerlinBtn";
            this.GeneratePerlinBtn.Size = new System.Drawing.Size(75, 23);
            this.GeneratePerlinBtn.TabIndex = 1;
            this.GeneratePerlinBtn.Text = "Generate";
            this.GeneratePerlinBtn.UseVisualStyleBackColor = true;
            this.GeneratePerlinBtn.Click += new System.EventHandler(this.button1_Click);
            // 
            // numericUpDown1
            // 
            this.numericUpDown1.DecimalPlaces = 1;
            this.numericUpDown1.Increment = new decimal(new int[] {
            1,
            0,
            0,
            131072});
            this.numericUpDown1.Location = new System.Drawing.Point(84, 6);
            this.numericUpDown1.Maximum = new decimal(new int[] {
            10,
            0,
            0,
            0});
            this.numericUpDown1.Name = "numericUpDown1";
            this.numericUpDown1.Size = new System.Drawing.Size(56, 21);
            this.numericUpDown1.TabIndex = 2;
            this.numericUpDown1.Value = new decimal(new int[] {
            25,
            0,
            0,
            131072});
            this.numericUpDown1.ValueChanged += new System.EventHandler(this.numericUpDown1_ValueChanged);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(6, 8);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(72, 12);
            this.label1.TabIndex = 3;
            this.label1.Text = "Persistence";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(48, 35);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(30, 12);
            this.label2.TabIndex = 4;
            this.label2.Text = "Size";
            // 
            // numericUpDown2
            // 
            this.numericUpDown2.DecimalPlaces = 2;
            this.numericUpDown2.Increment = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            this.numericUpDown2.Location = new System.Drawing.Point(84, 33);
            this.numericUpDown2.Maximum = new decimal(new int[] {
            2048,
            0,
            0,
            0});
            this.numericUpDown2.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            this.numericUpDown2.Name = "numericUpDown2";
            this.numericUpDown2.Size = new System.Drawing.Size(56, 21);
            this.numericUpDown2.TabIndex = 5;
            this.numericUpDown2.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(391, 28);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(100, 23);
            this.button1.TabIndex = 7;
            this.button1.Text = "Browse Folder";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click_1);
            // 
            // uvAnimSelectedPath
            // 
            this.uvAnimSelectedPath.Location = new System.Drawing.Point(121, 28);
            this.uvAnimSelectedPath.Name = "uvAnimSelectedPath";
            this.uvAnimSelectedPath.Size = new System.Drawing.Size(264, 21);
            this.uvAnimSelectedPath.TabIndex = 8;
            this.uvAnimSelectedPath.Text = "D:\\Projects\\fastbird-engine\\Data\\objects\\modules";
            // 
            // outFileName
            // 
            this.outFileName.Location = new System.Drawing.Point(121, 55);
            this.outFileName.Name = "outFileName";
            this.outFileName.Size = new System.Drawing.Size(264, 21);
            this.outFileName.TabIndex = 9;
            this.outFileName.Text = "Result.png";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(26, 58);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(89, 12);
            this.label3.TabIndex = 10;
            this.label3.Text = "Output Filepath";
            this.label3.Click += new System.EventHandler(this.label3_Click);
            // 
            // button2
            // 
            this.button2.Location = new System.Drawing.Point(121, 113);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(174, 23);
            this.button2.TabIndex = 11;
            this.button2.Text = "GenerateUVAnimTexture";
            this.button2.UseVisualStyleBackColor = true;
            this.button2.Click += new System.EventHandler(this.button2_Click);
            // 
            // uvFrames
            // 
            this.uvFrames.Increment = new decimal(new int[] {
            4,
            0,
            0,
            0});
            this.uvFrames.Location = new System.Drawing.Point(121, 86);
            this.uvFrames.Maximum = new decimal(new int[] {
            512,
            0,
            0,
            0});
            this.uvFrames.Name = "uvFrames";
            this.uvFrames.Size = new System.Drawing.Size(120, 21);
            this.uvFrames.TabIndex = 12;
            this.uvFrames.Value = new decimal(new int[] {
            16,
            0,
            0,
            0});
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(67, 88);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(48, 12);
            this.label4.TabIndex = 13;
            this.label4.Text = "Frames";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(31, 33);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(84, 12);
            this.label5.TabIndex = 14;
            this.label5.Text = "Source Folder";
            // 
            // outputFileBtn
            // 
            this.outputFileBtn.Location = new System.Drawing.Point(391, 53);
            this.outputFileBtn.Name = "outputFileBtn";
            this.outputFileBtn.Size = new System.Drawing.Size(100, 23);
            this.outputFileBtn.TabIndex = 15;
            this.outputFileBtn.Text = "Select File";
            this.outputFileBtn.UseVisualStyleBackColor = true;
            this.outputFileBtn.Click += new System.EventHandler(this.outputFileBtn_Click);
            // 
            // tabControl1
            // 
            this.tabControl1.Controls.Add(this.tabPage1);
            this.tabControl1.Controls.Add(this.tabPage2);
            this.tabControl1.Controls.Add(this.worldGenPage);
            this.tabControl1.Location = new System.Drawing.Point(12, 12);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(889, 630);
            this.tabControl1.TabIndex = 16;
            // 
            // tabPage1
            // 
            this.tabPage1.Controls.Add(this.perlinHeight);
            this.tabPage1.Controls.Add(this.perlinWidth);
            this.tabPage1.Controls.Add(this.label7);
            this.tabPage1.Controls.Add(this.label6);
            this.tabPage1.Controls.Add(this.pictureBox1);
            this.tabPage1.Controls.Add(this.GeneratePerlinBtn);
            this.tabPage1.Controls.Add(this.numericUpDown1);
            this.tabPage1.Controls.Add(this.label1);
            this.tabPage1.Controls.Add(this.label2);
            this.tabPage1.Controls.Add(this.numericUpDown2);
            this.tabPage1.Location = new System.Drawing.Point(4, 22);
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage1.Size = new System.Drawing.Size(881, 604);
            this.tabPage1.TabIndex = 0;
            this.tabPage1.Text = "RandomGenerator";
            this.tabPage1.UseVisualStyleBackColor = true;
            // 
            // perlinHeight
            // 
            this.perlinHeight.Increment = new decimal(new int[] {
            64,
            0,
            0,
            0});
            this.perlinHeight.Location = new System.Drawing.Point(207, 36);
            this.perlinHeight.Maximum = new decimal(new int[] {
            2048,
            0,
            0,
            0});
            this.perlinHeight.Name = "perlinHeight";
            this.perlinHeight.Size = new System.Drawing.Size(120, 21);
            this.perlinHeight.TabIndex = 9;
            this.perlinHeight.Value = new decimal(new int[] {
            256,
            0,
            0,
            0});
            // 
            // perlinWidth
            // 
            this.perlinWidth.Increment = new decimal(new int[] {
            64,
            0,
            0,
            0});
            this.perlinWidth.Location = new System.Drawing.Point(207, 8);
            this.perlinWidth.Maximum = new decimal(new int[] {
            2048,
            0,
            0,
            0});
            this.perlinWidth.Name = "perlinWidth";
            this.perlinWidth.Size = new System.Drawing.Size(120, 21);
            this.perlinWidth.TabIndex = 8;
            this.perlinWidth.Value = new decimal(new int[] {
            256,
            0,
            0,
            0});
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(161, 36);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(40, 12);
            this.label7.TabIndex = 7;
            this.label7.Text = "Height";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(166, 8);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(35, 12);
            this.label6.TabIndex = 6;
            this.label6.Text = "Width";
            // 
            // tabPage2
            // 
            this.tabPage2.Controls.Add(this.btnGenModulePack);
            this.tabPage2.Controls.Add(this.button2);
            this.tabPage2.Controls.Add(this.outputFileBtn);
            this.tabPage2.Controls.Add(this.button1);
            this.tabPage2.Controls.Add(this.label5);
            this.tabPage2.Controls.Add(this.uvAnimSelectedPath);
            this.tabPage2.Controls.Add(this.label4);
            this.tabPage2.Controls.Add(this.outFileName);
            this.tabPage2.Controls.Add(this.uvFrames);
            this.tabPage2.Controls.Add(this.label3);
            this.tabPage2.Location = new System.Drawing.Point(4, 22);
            this.tabPage2.Name = "tabPage2";
            this.tabPage2.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage2.Size = new System.Drawing.Size(881, 604);
            this.tabPage2.TabIndex = 1;
            this.tabPage2.Text = "UV Anim map";
            this.tabPage2.UseVisualStyleBackColor = true;
            // 
            // worldGenPage
            // 
            this.worldGenPage.Controls.Add(this.worldPictureBox);
            this.worldGenPage.Controls.Add(this.genWorldBtn);
            this.worldGenPage.Location = new System.Drawing.Point(4, 22);
            this.worldGenPage.Name = "worldGenPage";
            this.worldGenPage.Size = new System.Drawing.Size(881, 604);
            this.worldGenPage.TabIndex = 2;
            this.worldGenPage.Text = "WorldGen";
            this.worldGenPage.UseVisualStyleBackColor = true;
            // 
            // worldPictureBox
            // 
            this.worldPictureBox.Location = new System.Drawing.Point(3, 32);
            this.worldPictureBox.Name = "worldPictureBox";
            this.worldPictureBox.Size = new System.Drawing.Size(875, 569);
            this.worldPictureBox.TabIndex = 1;
            this.worldPictureBox.TabStop = false;
            // 
            // genWorldBtn
            // 
            this.genWorldBtn.Location = new System.Drawing.Point(3, 3);
            this.genWorldBtn.Name = "genWorldBtn";
            this.genWorldBtn.Size = new System.Drawing.Size(110, 23);
            this.genWorldBtn.TabIndex = 0;
            this.genWorldBtn.Text = "GenerateWorld";
            this.genWorldBtn.UseVisualStyleBackColor = true;
            this.genWorldBtn.Click += new System.EventHandler(this.genWorldBtn_Click);
            // 
            // btnGenModulePack
            // 
            this.btnGenModulePack.Location = new System.Drawing.Point(310, 113);
            this.btnGenModulePack.Name = "btnGenModulePack";
            this.btnGenModulePack.Size = new System.Drawing.Size(161, 23);
            this.btnGenModulePack.TabIndex = 16;
            this.btnGenModulePack.Text = "Generate Module Pack";
            this.btnGenModulePack.UseVisualStyleBackColor = true;
            this.btnGenModulePack.Click += new System.EventHandler(this.btnGenModulePack_Click);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(913, 654);
            this.Controls.Add(this.tabControl1);
            this.Name = "Form1";
            this.Text = "Fastbird Engine";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Form1_FormClosing);
            this.Load += new System.EventHandler(this.Form1_Load);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown1)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown2)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.uvFrames)).EndInit();
            this.tabControl1.ResumeLayout(false);
            this.tabPage1.ResumeLayout(false);
            this.tabPage1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.perlinHeight)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.perlinWidth)).EndInit();
            this.tabPage2.ResumeLayout(false);
            this.tabPage2.PerformLayout();
            this.worldGenPage.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.worldPictureBox)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.PictureBox pictureBox1;
        private System.Windows.Forms.Button GeneratePerlinBtn;
        private System.Windows.Forms.NumericUpDown numericUpDown1;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.NumericUpDown numericUpDown2;
        private System.Windows.Forms.FolderBrowserDialog folderBrowserDialog1;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.TextBox uvAnimSelectedPath;
        private System.Windows.Forms.TextBox outFileName;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Button button2;
        private System.Windows.Forms.NumericUpDown uvFrames;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Button outputFileBtn;
        private System.Windows.Forms.SaveFileDialog saveFileDialog1;
        private System.Windows.Forms.TabControl tabControl1;
        private System.Windows.Forms.TabPage tabPage1;
        private System.Windows.Forms.TabPage tabPage2;
        private System.Windows.Forms.NumericUpDown perlinHeight;
        private System.Windows.Forms.NumericUpDown perlinWidth;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.TabPage worldGenPage;
        private System.Windows.Forms.PictureBox worldPictureBox;
        private System.Windows.Forms.Button genWorldBtn;
        private System.Windows.Forms.Button btnGenModulePack;
    }
}

