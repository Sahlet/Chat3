namespace chat_client
{
    partial class Conference
    {
        /// <summary> 
        /// Требуется переменная конструктора.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary> 
        /// Освободить все используемые ресурсы.
        /// </summary>
        /// <param name="disposing">истинно, если управляемый ресурс должен быть удален; иначе ложно.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Код, автоматически созданный конструктором компонентов

        /// <summary> 
        /// Обязательный метод для поддержки конструктора - не изменяйте 
        /// содержимое данного метода при помощи редактора кода.
        /// </summary>
        protected void InitializeComponent()
        {
            this.panel1 = new System.Windows.Forms.Panel();
            this.name = new System.Windows.Forms.Label();
            this.panel1.SuspendLayout();
            this.SuspendLayout();
            // 
            // panel1
            // 
            this.panel1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.panel1.BackColor = System.Drawing.SystemColors.ControlLight;
            this.panel1.Controls.Add(this.name);
            this.panel1.Location = new System.Drawing.Point(0, 1);
            this.panel1.MinimumSize = new System.Drawing.Size(35, 15);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(35, 15);
            this.panel1.TabIndex = 20;
            this.panel1.Click += new System.EventHandler(this.Contact_Click);
            // 
            // name
            // 
            this.name.AutoEllipsis = true;
            this.name.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F);
            this.name.ForeColor = System.Drawing.SystemColors.HotTrack;
            this.name.ImeMode = System.Windows.Forms.ImeMode.NoControl;
            this.name.Location = new System.Drawing.Point(0, 0);
            this.name.MaximumSize = new System.Drawing.Size(0, 15);
            this.name.MinimumSize = new System.Drawing.Size(35, 15);
            this.name.Name = "name";
            this.name.Size = new System.Drawing.Size(35, 15);
            this.name.TabIndex = 20;
            this.name.Text = "имя";
            this.name.Click += new System.EventHandler(this.Contact_Click);
            // 
            // Conference
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.Transparent;
            this.Controls.Add(this.panel1);
            this.MaximumSize = new System.Drawing.Size(0, 17);
            this.MinimumSize = new System.Drawing.Size(35, 17);
            this.Name = "Conference";
            this.Size = new System.Drawing.Size(35, 17);
            this.Click += new System.EventHandler(this.Contact_Click);
            this.panel1.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        protected System.Windows.Forms.Panel panel1;
        public System.Windows.Forms.Label name;


    }
}
