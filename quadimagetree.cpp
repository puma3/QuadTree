#include "quadimagetree.h"

QuadImageTree::QuadImageTree()
{}

void QuadImageTree::loadImage(const QString &fileName)
{
    m_picture.load(fileName);

    m_size = m_picture.size();
}

void QuadImageTree::convert2Grayscale()
{
    qDebug() << "Image conversion to grayscale" << endl;
    m_picture = m_picture.convertToFormat(QImage::Format_Grayscale8);
    m_size = m_picture.size();
}

bool QuadImageTree::isInvariant(int _x0, int _xf, int _y0, int _yf, float _threshold)
{
    float avg = 0.0,
          var = 0.0,
          diff = 0.0;
    size_t size = (_xf-_x0)*(_yf-_y0);
    if(size < 2)
        return true;

//    qDebug() << "Size: " << size << endl;

/////////////////  AVERAGE  /////////////////
    for(int _x = _x0;_x < _xf; _x++)
        for(int _y = _y0;_y < _yf; _y++)
            avg += m_picture.pixelColor(_x,_y).blue();
    avg /= size;
//    qDebug() << "Average: " << avg << endl;

/////////////////  VARIANCE /////////////////
    for(int _x = _x0;_x < _xf; _x++)
        for(int _y = _y0;_y < _yf; _y++) {
            diff = m_picture.pixelColor(_x,_y).blue() - avg;
            diff *= diff;
            var += diff;
        }
    var /= size;
    var = sqrt(var);                                //Std deviation
//    qDebug() << "Variance: " << var << endl;
//    qDebug() << "var: " << var << "\tthreshold: " << _threshold << "\tdx: " << _xf-_x0 << "\tdy: " << _yf-_y0 << endl;

    return var > _threshold ? false : true;

/////////////////FORMER CODE/////////////////
//    QRgb _color0 = m_picture.pixel(_x0,_y0);

//    for(int _x = _x0;_x < _xf; _x++)
//        for(int _y = _y0;_y < _yf; _y++)
//            if(m_picture.pixel(_x,_y) != _color0)
//                return false;

//    return true;
}

void QuadImageTree::buildQTree(float _threshold)
{
    buildQTree(&m_node, 0, m_size.width(), 0, m_size.height(), _threshold);
    qDebug() << "Called with threshold: " << _threshold << endl;
}

void QuadImageTree::buildQTree(QuadImageNode *ptr, int _x0, int _xf, int _y0, int _yf, float _threshold)
{
    int     _half_x = (_xf - _x0) / 2,
            _half_y = (_yf - _y0) / 2,
            _new_half_x = _x0 + _half_x,
            _new_half_y = _y0 + _half_y;

    if(isInvariant(_x0, _xf, _y0, _yf, _threshold)) {
        ptr->m_color = m_picture.pixel(_new_half_x,_new_half_y);
//        qDebug() << "Finished building branch!" << endl;
        return;
    }

    for(int i = 0; i < 4; i++)
        ptr->m_children[i] = new QuadImageNode();

    buildQTree(ptr->m_children[0], _x0, _new_half_x, _y0, _new_half_y, _threshold);
    buildQTree(ptr->m_children[1], _new_half_x, _xf, _y0, _new_half_y, _threshold);
    buildQTree(ptr->m_children[2], _x0, _new_half_x, _new_half_y, _yf, _threshold);
    buildQTree(ptr->m_children[3], _new_half_x, _xf, _new_half_y, _yf, _threshold);
}

void QuadImageTree::loadQTree()
{
    m_picture.fill(Qt::transparent);
    QTreeImage(&m_node, 0, m_size.width(), 0, m_size.height());

//    Guarda el árbol en el disco
    //saveQTree();
}

void QuadImageTree::saveQTree()
{
    QFile treeOnDisk("arbol.QT");
    if(!treeOnDisk.open(QIODevice::ReadWrite))
        return;
    QDataStream out(&treeOnDisk);
    out << m_size;

////////////////////////////////    DEBUG
//    cout << "SIZE_wr:" << m_size.height() << " " << m_size.width() << endl;
////////////////////////////////
    QTree2File(&m_node, out);
}

void QuadImageTree::loadQTreeFromFile()
{
    //////////////////////////////    DEBUG
//        cout << "\nOPENING FILE\n_________________________________\n\n";
    //////////////////////////////
    QFile treeOnDisk("arbol.QT");
    if(!treeOnDisk.open(QIODevice::ReadWrite))
        return;
    QDataStream in(&treeOnDisk);
    in >> m_size;
    //////////////////////////////    DEBUG
//        cout << m_size.height() << " " << m_size.width() << endl;
    //////////////////////////////
    QTreeFromFile(&m_node, in);
    m_picture.fill(Qt::transparent);
    m_picture.scaled(m_size);
    QTreeImage(&m_node, 0, m_size.width(), 0, m_size.height());
}


//Repinta m_picture basado en QTree
void QuadImageTree::QTreeImage(QuadImageNode *ptr, int _x0, int _xf, int _y0, int _yf)
{
    //OPTIMIZAR ESTA PARTE
    bool hasChildren = true;
    for(int i = 0; i < 4; i++)
        if(!ptr->m_children[i])
            hasChildren = false;
    ///////////////////////////////

    if(!hasChildren) {
        for(int _x = _x0; _x < _xf; _x++)
            for(int _y = _y0; _y < _yf; _y++)
                m_picture.setPixel(_x, _y, ptr->m_color);

        for(int _x = _x0; _x < _xf; _x++) {
            m_picture.setPixel(_x, _y0, 0x00ff0c);
            //m_picture.setPixel(_x, _yf, 0x00ff0c);
        }

        for(int _y = _y0; _y < _yf; _y++) {
            m_picture.setPixel(_x0, _y, 0x00ff0c);
            //m_picture.setPixel(_xf, _y, 0x00ff0c);
        }

        return;
    }

    int     _half_x = (_xf - _x0) / 2,
            _half_y = (_yf - _y0) / 2,
            _new_half_x = _x0 + _half_x,
            _new_half_y = _y0 + _half_y;

    QTreeImage(ptr->m_children[0], _x0, _new_half_x, _y0, _new_half_y);
    QTreeImage(ptr->m_children[1], _new_half_x, _xf, _y0, _new_half_y);
    QTreeImage(ptr->m_children[2], _x0, _new_half_x, _new_half_y, _yf);
    QTreeImage(ptr->m_children[3], _new_half_x, _xf, _new_half_y, _yf);
}

void QuadImageTree::QTree2File(QuadImageNode *ptr, QDataStream &out)
{
    if(!ptr) {
        out << 0x000000;
        //////////////////////////////    DEBUG
//        cout << 0x000000 << endl;
        //////////////////////////////
        return;
    }

    out << ptr->m_color;
    //////////////////////////////    DEBUG
//    cout << ptr->m_color << endl;
    //////////////////////////////

    for(int i = 0; i < 4; i++)
        QTree2File(ptr->m_children[i], out);
}

void QuadImageTree::QTreeFromFile(QuadImageNode *ptr, QDataStream &in)
{
    QRgb color;
    in >> color;

    //////////////////////////////    DEBUG
//    cout << color << ": ";
    //////////////////////////////

    if(color == 0x000000) {
        ptr = NULL;
        return;
    }

    if(!ptr)
        ptr = new QuadImageNode();

    ptr->m_color = color;

    //////////////////////////////    DEBUG
//    cout << ptr->m_color << endl;
    //////////////////////////////

    for(int i = 0; i < 4; i++) {
        //////////////////////////////    DEBUG
//        cout << "Child [" << i << "] will be called, color: ";
        //////////////////////////////
        QTree2File(ptr->m_children[i], in);
    }
}

void QuadImageTree::paint(QLabel *_drawer)
{
    _drawer->setPixmap(QPixmap::fromImage(m_picture));
}

QuadImageNode::QuadImageNode()
{
    for(int i = 0; i < 4; i++)
        m_children[i] = NULL;
}

QuadImageNode::~QuadImageNode()
{
    for(int i = 0; i < 4; i++)
        if(m_children[i])
            delete m_children[i];
}
