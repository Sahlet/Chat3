using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace chat_client
{
    public partial class Button_more : UserControl
    {
        method do_some;
        public Button_more(method do_)
        {
            InitializeComponent();
            do_some = do_;
        }

        private void more_Click(object sender, EventArgs e)
        {
            do_some(sender, e);
        }
    }
}
