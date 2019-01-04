/**
* This file is part of ORB-SLAM2.
*
* Copyright (C) 2014-2016 Raúl Mur-Artal <raulmur at unizar dot es> (University of Zaragoza)
* For more information see <https://github.com/raulmur/ORB_SLAM2>
*
* ORB-SLAM2 is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* ORB-SLAM2 is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with ORB-SLAM2. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ORBEXTRACTOR_H
#define ORBEXTRACTOR_H

#include <vector>
#include <list>
#include <opencv/cv.h>


//这个文件主要负责进行ORB特征点的提取和数目分配功能


namespace ORB_SLAM2
{

/**
 * @brief 提取器节点
 * @details 用于在特征点的分配过程中。
 * 
 */
class ExtractorNode
{
public:
	//类的构造函数
    ExtractorNode():bNoMore(false){}

    //在八叉树分配特征点的过程中，实现一个节点分裂为4个节点的操作
    void DivideNode(ExtractorNode &n1, ExtractorNode &n2, ExtractorNode &n3, ExtractorNode &n4);

	//保存有当前节点的特征点计数
    std::vector<cv::KeyPoint> vKeys;
	//当前节点所对应的图像坐标边界
    cv::Point2i UL, UR, BL, BR;
	//存储提取器节点的列表（其实就是双向链表）的一个迭代器,可以参考[http://www.runoob.com/cplusplus/cpp-overloading.html]
	//这个迭代器提供了访问总节点列表的方式，需要结合cpp文件进行分析
    std::list<ExtractorNode>::iterator lit;
	
	//如果节点中只有一个特征点的话，说明这个节点不能够再进行分裂了，这个标志置位
	//如果你要问这个节点中如果没有特征点的话怎么办，我只想说那样的话这个节点就直接被删除了
    bool bNoMore;
};



/**
 * @brief ORB特征点提取器
 * 
 */
class ORBextractor
{
public:
    
	//定义一个枚举类型用于表示使用HARRIS响应值还是使用FAST响应值
	//TODO 但是在程序中好像并没有被用到
    enum {HARRIS_SCORE=0, FAST_SCORE=1 };

	//构造函数
    ORBextractor(int nfeatures,				//指定要提取出来的特征点数目
				 float scaleFactor,			//于图像金字塔有关的缩放系数
				 int nlevels,				//指定需要提取特征点的图像金字塔层
                 int iniThFAST,				//初始的默认FAST响应值阈值
				 int minThFAST);			//较小的FAST响应值阈值
	//上面之所以会有两种响应值的阈值，原因是，程序先使用初始的默认FAST响应值阈值提取图像cell中的特征点；如果提取到的
	//特征点数目不足，那么就降低要求，使用较小FAST响应值阈值进行再次提取，以获得尽可能多的FAST角点。

	//这个类的析构函数
    ~ORBextractor(){}

    // Compute the ORB features and descriptors on an image.
    // ORB are dispersed on the image using an octree.
    //
    // Mask is ignored in the current implementation. 的确是，函数中实际上并没有用到MASK。
    //TODO 这是一个括号运算符的重载定义，我想看看是怎么调用这个函数的
    /**
     * @brief 使用八叉树的方法将提取到的ORB特征点尽可能均匀地分布在整个图像中
     * @details 这里是重载了这个ORBextractor类的括号运算符
     * 
     * @param image         要操作的图像
     * @param mask          图像掩膜，辅助进行图片处理，可以参考[https://www.cnblogs.com/skyfsm/p/6894685.html]
     * @param keypoints     保存提取出来的特征点的向量
     * @param descriptors   输出用的保存特征点描述子的cv::Mat
     */
    void operator()( 
				cv::InputArray image, 					//要操作的图像
				cv::InputArray mask,					//图像掩膜，辅助进行图片处理，可以参考[https://www.cnblogs.com/skyfsm/p/6894685.html]
				std::vector<cv::KeyPoint>& keypoints,	//保存提取出来的特征点的向量
				cv::OutputArray descriptors);			//输出用的保存特征点描述子的cv::Mat

	//下面的这些内联函数都是用来直接获取类的成员变量的
	
	//获取当前提取器所在的图像金字塔层
    int inline GetLevels(){
        return nlevels;}

    //获取当前提取器所在的图像的缩放因子，这个不带s的因子表示是相临近层之间的
    float inline GetScaleFactor(){
        return scaleFactor;}

    //获取图像金字塔中每个图层相对于底层图像的缩放因子
    std::vector<float> inline GetScaleFactors(){
        return mvScaleFactor;
    }

    //获取上面的那个缩放因子s的倒数
    std::vector<float> inline GetInverseScaleFactors(){
        return mvInvScaleFactor;
    }
    
	//sigma^2就是每层图像相对于初始图像缩放因子的平方，参考cpp文件中类构造函数的操作
    std::vector<float> inline GetScaleSigmaSquares(){
        return mvLevelSigma2;
    }

    //获取上面sigma平方的倒数
    std::vector<float> inline GetInverseScaleSigmaSquares(){
        return mvInvLevelSigma2;
    }

    //这个是用来存储图像金字塔的变量，一个元素存储一层图像
    std::vector<cv::Mat> mvImagePyramid;

protected:
	
	//下面这些都是保护成员哟

	//针对给出的一张图像，计算其图像金字塔
    void ComputePyramid(cv::Mat image);
	//以八叉树分配特征点的方式，计算图像金字塔中的特征点
	//这里两层vector的意思可能是，第一层存储的是某张图片中的所有特征点，
	//而第二层则是存储图像金字塔中所有图像的vectors of keypoints
    void ComputeKeyPointsOctTree(std::vector<std::vector<cv::KeyPoint> >& allKeypoints);    
	//对于某一图层，分配其特征点，通过八叉树的方式
    std::vector<cv::KeyPoint> DistributeOctTree(
		const std::vector<cv::KeyPoint>& vToDistributeKeys, 	//等待分配的特征点
		const int &minX,		//分发的图像范围
		const int &maxX, 
		const int &minY, 
		const int &maxY, 
		const int &nFeatures, 	//设定的、本图层中想要提取的特征点数目
		const int &level);		//要提取的图像所在的金字塔层
	//这是使用另外一种老办法提取并平均特征点的方法，但是在实际的程序中并没有用到
    void ComputeKeyPointsOld(std::vector<std::vector<cv::KeyPoint> >& allKeypoints);
	//用于计算描述子的随机采样点集合
    std::vector<cv::Point> pattern;

	//NOTE 作者不地道啊，这里是类的成员变量，说好的变量名的m前缀呢？
    int nfeatures;			//整个图像金字塔中，要提取的特征点数目
    double scaleFactor;		//图像金字塔层与层之间的缩放因子
    int nlevels;			//图像金字塔的层数
    int iniThFAST;			//初始的FAST响应值阈值
    int minThFAST;			//最小的FAST响应值阈值

    std::vector<int> mnFeaturesPerLevel;		//分配到每层图像中，要提取的特征点数目

    std::vector<int> umax;	//计算描述子的时候，有个圆形的图像区域，这个vector中存储了每行u轴的边界（四分之一，其他部分通过对称获得）

    std::vector<float> mvScaleFactor;		//每层图像的缩放因子
    std::vector<float> mvInvScaleFactor;    //以及每层缩放因子的倒数
    std::vector<float> mvLevelSigma2;		//存储每层的sigma^2,即上面每层图像相对于底层图像缩放倍数的平方
    std::vector<float> mvInvLevelSigma2;	//sigma平方的倒数
};

} //namespace ORB_SLAM

#endif

