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
    public partial class ItemList_with_Button_more : UserControl
    {
        private method load_more_to_this;
        public ItemList_with_Button_more(method load_more_to_this, DockStyle dockStyle, DockStyle Button_more_dockStyle){
            InitializeComponent();
            more = new Button_more(new method(this.more_Click));
            more.Dock = Button_more_dockStyle;
            this.Dock = dockStyle;
            this.Controls.Add(more);
            this.load_more_to_this = load_more_to_this;
        }
        //очищает компонент от когтроллов, и добавляет свой Button_more
        public void Clear(){
            this.Controls.Clear();
            this.Controls.Add(more);
        }
        public int get_n_visible_children() {
            int res = 0;
            foreach (Control obj in Controls) {
                if (obj.Visible) res++;
            }
            return res;
        }
        private void more_Click(object sender, EventArgs e) {
            load_more_to_this(this, e);
        }
    }
}
