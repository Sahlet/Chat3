namespace chat_client
{
    partial class Button_more
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
        private void InitializeComponent()
        {
            this.more = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // more
            // 
            this.more.Anchor = System.Windows.Forms.AnchorStyles.None;
            this.more.Font = new System.Drawing.Font("Microsoft Sans Serif", 12F);
            this.more.ImeMode = System.Windows.Forms.ImeMode.NoControl;
            this.more.Location = new System.Drawing.Point(0, 1);
            this.more.Name = "more";
            this.more.Size = new System.Drawing.Size(75, 30);
            this.more.TabIndex = 17;
            this.more.Text = "Еще";
            this.more.UseVisualStyleBackColor = true;
            this.more.Click += new System.EventHandler(this.more_Click);
            // 
            // Button_more
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.Transparent;
            this.Controls.Add(this.more);
            this.MaximumSize = new System.Drawing.Size(0, 32);
            this.MinimumSize = new System.Drawing.Size(75, 32);
            this.Name = "Button_more";
            this.Size = new System.Drawing.Size(75, 32);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button more;
    }
}
