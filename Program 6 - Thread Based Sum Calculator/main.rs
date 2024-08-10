/*
Author:         Santosh Ramesh
Email:          rameshsa@oregonstate.edu
Date:           3-13-21
Description:    This program uses Rust's threads to partition numbers into seperate threads to calculate a final sum value
*/

use std::env; // to get arugments passed to the program
use std::thread;

/*
* Print the number of partitions and the size of each partition
* @param vs A vector of vectors
*/
fn print_partition_info(vs: &Vec<Vec<usize>>){
    println!("Number of partitions = {}", vs.len());
    for i in 0..vs.len(){
        println!("\tsize of partition {} = {}", i, vs[i].len());
    }
}

/*
* Create a vector with integers from 0 to num_elements -1
* @param num_elements How many integers to generate
* @return A vector with integers from 0 to (num_elements - 1)
*/
fn generate_data(num_elements: usize) -> Vec<usize>{
    let mut v : Vec<usize> = Vec::new();
    for i in 0..num_elements {
        v.push(i);
    }
    return v;
}

/*
* Partition the data in the vector v into 2 vectors
* @param v Vector of integers
* @return A vector that contains 2 vectors of integers

*/
fn partition_data_in_two(v: &Vec<usize>) -> Vec<Vec<usize>>{
    let partition_size = v.len() / 2;
    // Create a vector that will contain vectors of integers
    let mut xs: Vec<Vec<usize>> = Vec::new();

    // Create the first vector of integers
    let mut x1 : Vec<usize> = Vec::new();
    // Add the first half of the integers in the input vector to x1
    for i in 0..partition_size{
        x1.push(v[i]);
    }
    // Add x1 to the vector that will be returned by this function
    xs.push(x1);

    // Create the second vector of integers
    let mut x2 : Vec<usize> = Vec::new();
    // Add the second half of the integers in the input vector to x2
    for i in partition_size..v.len(){
        x2.push(v[i]);
    }
    // Add x2 to the vector that will be returned by this function
    xs.push(x2);
    // Return the result vector
    xs
}

/*
* Sum up the all the integers in the given vector
* @param v Vector of integers
* @return Sum of integers in v
* Note: this function has the same code as the reduce_data function.
*       But don't change the code of map_data or reduce_data.
*/
fn map_data(v: &Vec<usize>) -> usize{
    let mut sum = 0;
    for i in v{
        sum += i;
    }
    sum
}

/*
* Sum up the all the integers in the given vector
* @param v Vector of integers
* @return Sum of integers in v
*/
fn reduce_data(v: &Vec<usize>) -> usize{
    let mut sum = 0;
    for i in v{
        sum += i;
    }
    sum
}

/*
* A single threaded map-reduce program
*/
fn main() {

    // Use std::env to get arguments passed to the program
    let args: Vec<String> = env::args().collect();
    if args.len() != 3 {
        println!("ERROR: Usage {} num_partitions num_elements", args[0]);
        return;
    }
    let num_partitions : usize = args[1].parse().unwrap();
    let num_elements : usize = args[2].parse().unwrap();
    if num_partitions < 1{
      println!("ERROR: num_partitions must be at least 1");
        return;
    }
    if num_elements < num_partitions{
        println!("ERROR: num_elements cannot be smaller than num_partitions");
        return;
    }

    // Generate data.
    let v = generate_data(num_elements);

    // PARTITION STEP: partition the data into 2 partitions
    let xs = partition_data_in_two(&v);

    // Print info about the partitions
    print_partition_info(&xs);

    let mut intermediate_sums : Vec<usize> = Vec::new();

    // MAP STEP: Process each partition

    // CHANGE CODE START: Don't change any code above this line

    // Change the following code to create 2 threads that run concurrently and each of which uses map_data() function to process one of the two partitions

    let v1 = xs[0].clone();
    let v2 = xs[1].clone();

    let p1 = thread::spawn(move || map_data(&v1));
    let p2 = thread::spawn(move || map_data(&v2));

    let s1 = p1.join().unwrap();
    let s2 = p2.join().unwrap();

    intermediate_sums.push(s1);
    intermediate_sums.push(s2);

    // CHANGE CODE END: Don't change any code below this line until the next CHANGE CODE comment

    // Print the vector with the intermediate sums
    println!("Intermediate sums = {:?}", intermediate_sums);

    // REDUCE STEP: Process the intermediate result to produce the final result
    let sum = reduce_data(&intermediate_sums);
    println!("Sum = {}", sum);

    // CHANGE CODE: Add code that does the following:
    let m = generate_data(num_elements);
    let mut intermediate_sums : Vec<usize> = Vec::new();

    // 1. Calls partition_data to partition the data into equal partitions
    let ms = partition_data(num_partitions, &m);

    // 2. Calls print_partition_info to print info on the partitions that have been created
    print_partition_info(&ms);

    // 3. Creates one thread per partition and uses each thread to concurrently process one partition
    let mut threads = vec![];   // holds all of the threads
    for i in 0..num_partitions{
        let vn = ms[i].clone();
        let p = thread::spawn(move || map_data(&vn));
        threads.push(p)
    }

    // 4. Collects the intermediate sums from all the threads
    for current_thread in threads {
        // Wait for the thread to finish. Returns a result.
        let s = current_thread.join().unwrap();
        intermediate_sums.push(s);
    }
    
    // 5. Prints information about the intermediate sums
    println!("Intermediate sums = {:?}", intermediate_sums);

    // 5. Calls reduce_data to process the intermediate sums
    let sum = reduce_data(&intermediate_sums);

    // 6. Prints the final sum computed by reduce_data
    println!("Sum = {}", sum);
}

/*
* CHANGE CODE: code this function
* Note: Don't change the signature of this function
*
* Partitions the data into a number of partitions such that
* - the returned partitions contain all elements that are in the input vector
* - if num_elements is a multiple of num_partitions, then all partitions must have equal number of elements
* - if num_elements is not a multiple of num_partitions, some partitions can have one more element than other partitions
*
* @param num_partitions The number of partitions to create
* @param v The data to be partitioned
* @return A vector that contains vectors of integers
* 
*/
fn partition_data(num_partitions: usize, v: &Vec<usize>) -> Vec<Vec<usize>>{
    // Remove the following line which has been added to remove a compiler error
    let partition_size = v.len() / num_partitions;
    let mut reminder = v.len() % num_partitions;
    let mut index = 0;

    // Create a vector that will contain vectors of integers
    let mut xs: Vec<Vec<usize>> = Vec::new();

    // Pushing into vector
    for _i in 0..num_partitions{
        // Creating current vector to be utilized in for loops for pushing
        let mut cv : Vec<usize> = Vec::new();

        if reminder > 0 {
            reminder = reminder - 1;
            for _j in 0..(partition_size + 1){
                cv.push(v[index]);
                index = index + 1;
            }

        } else{
            for _j in 0..partition_size{
                cv.push(v[index]);
                index = index + 1;
            }
        }
        xs.push(cv);
    }

    // Return the result vector
    xs
}
