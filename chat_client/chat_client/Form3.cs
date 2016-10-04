using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace chat_client
{
    public partial class Form3 : Form
    {
        public Form3(Control first, Control second){
            InitializeComponent();
            panel1.Controls.Add(first);
            if ((Label)second != null)
            {
                second.MaximumSize = new Size(this.Size.Width, 0);
                this.AutoSize = second.AutoSize;
            }
            panel2.Controls.Add(second);
        }
    }
}
